/*
 * p4-validation-model: Reference model for P4 programs in the PFPSim Framework
 *
 * Copyright (C) 2016 Concordia Univ., Montreal
 *     Samar Abdi
 *     Umair Aftab
 *     Gordon Bailey
 *     Faras Dewal
 *     Shafigh Parsazad
 *     Eric Tremblay
 *
 * Copyright (C) 2016 Ericsson
 *     Bochra Boughzala
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "./P4Switch.h"
#include <string>

#include "common/P4.h"
#include "common/InputStimulus.h"
#include "common/Packet.h"

P4Switch::P4Switch(sc_module_name nm, pfp::core::PFPObject* parent,std::string configfile ):P4SwitchSIM(nm,parent,configfile) {  // NOLINT
    /*sc_spawn threads*/
  sc_spawn(sc_bind(&P4Switch::command_processing_thread, this));
  sc_spawn(sc_bind(&P4Switch::P4Switch_PortServiceThread, this));
}

void P4Switch::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
  P4::get("npu")->init_objects(SPARG("p4"));
}
void P4Switch::P4Switch_PortServiceThread() {
  auto p4       = P4::get("npu");
  auto ingress  = p4->get_pipeline("ingress");
  auto egress   = p4->get_pipeline("egress");
  auto parser   = p4->get_parser("parser");
  auto deparser = p4->get_deparser("deparser");
  while (true) {
    // Splitter
    // 1. Read data from stimulus via ingress
    auto received_pkt = in->get();
    auto recvpkt_obj = std::dynamic_pointer_cast
                            <InputStimulus>(received_pkt);
    auto received = recvpkt_obj->stimulus_object();

    auto received_packet = std::get<0>(received);

    std::vector<char> raw_header;
    // 2. Split packet payload and header
    auto start_of_header = received_packet->data().begin();
    // start_of_header + header_length;
    auto end_of_header   = received_packet->data().end();
    // 2.1 Extract the header part of the packet into a new buffer
    std::copy (start_of_header, end_of_header, std::back_inserter(raw_header));

    // Parser
    auto packet_header = p4->
        new_packet_ptr(
           0/*ingress_port*/, 0,
           raw_header.size(),
           bm::PacketBuffer(
              raw_header.size(),  // capacity (TODO(gordon) be less arbitrary)
              raw_header.data(),
              raw_header.size()));

    parser->parse(packet_header.get());

    // App layer
    auto p4_packet = packet_header.get();
    p4->lock.read_lock();
    ingress->apply(p4_packet);
    // TODO(gordon) if( ! np.drop()){
    egress->apply(p4_packet);
    p4->lock.read_unlock();


    // Deparser
    std::vector<uint8_t> rh;
    deparser->deparse(packet_header.get());
    char    * buf = packet_header->data();
    size_t    len = packet_header->get_data_size();

    std::copy(buf, buf + len, std::back_inserter(rh));

    auto outp = std::make_shared<Packet>(0,0,0,rh);


    out->put(outp);
  }
  // Thread function to service input ports.
}
std::shared_ptr<pfp::cp::CommandResult>
P4Switch::send_command(const std::shared_ptr<pfp::cp::Command> & cmd) {
  npulog(profile, cout << "send command start" << endl;)
  current_command = cmd;

  npulog(profile, cout << "notifying new_command" << endl;)
  new_command.notify();

  npulog(profile, cout << "waiting command_processed" << endl;)
  wait(command_processed);

  return current_result;
}

void P4Switch::command_processing_thread() {
  while (true) {
    npulog(profile, cout << "waiting new_command" << endl;)
    wait(new_command);

    npulog(profile, cout << "accepting command" << endl;)
    current_result = accept_command(current_command);

    npulog(profile, cout << "notifying command_processed" << endl;)
    command_processed.notify();
  }
}

/* anonymous */namespace {

std::string to_string(const pfp::cp::Bytes & b) {
  return std::string((const char*)b.data(), b.size());
}

bm::MatchKeyParam to_p4_key(const pfp::cp::MatchKey * k) {
  switch (k->get_type()) {
    case pfp::cp::MatchKey::Type::EXACT:
      return bm::MatchKeyParam(
          bm::MatchKeyParam::Type::EXACT,
          to_string(k->get_data()));

    case pfp::cp::MatchKey::Type::LPM:
      return bm::MatchKeyParam(
          bm::MatchKeyParam::Type::LPM,
          to_string(k->get_data()),
          k->get_prefix_len());

    case pfp::cp::MatchKey::Type::TERNARY:
      return bm::MatchKeyParam(
          bm::MatchKeyParam::Type::TERNARY,
          to_string(k->get_data()),
          to_string(k->get_mask()));

    default:
      std::cerr << "Unknown MatchKey::Type " << k->get_type() << std::endl;
      assert(false);
  }
}

}  // end anonymous namespace

std::shared_ptr<pfp::cp::CommandResult>
P4Switch::process(pfp::cp::InsertCommand * cmd) {
  cout << "Insert Command at P4Switch" << endl;

  // Convert all of the match keys
  std::vector<bm::MatchKeyParam> keys;
  for (const auto & key : cmd->get_keys()) {
    keys.push_back(to_p4_key(key.get()));
  }

  // Convert all of the action parameters
  bm::ActionData action_data;
  for (const pfp::cp::Bytes & b : cmd->get_action().get_params()) {
    action_data.push_back_action_data
      (bm::Data((const char *)b.data(), b.size()));
  }

  // Insert the entry!
  bm::entry_handle_t handle;
  auto p4 = P4::get("npu");
  p4->lock.write_lock();
  bm::MatchErrorCode rc = p4->mt_add_entry(0,
        cmd->get_table_name(),  keys,
        cmd->get_action().get_name(), action_data,
        &handle);
  p4->lock.write_unlock();

  return cmd->success_result(handle);
}

std::shared_ptr<pfp::cp::CommandResult>
P4Switch::process(pfp::cp::ModifyCommand *cmd) {
  cout << "Modify Command at P4Switch" << endl;
  return cmd->success_result();
}
std::shared_ptr<pfp::cp::CommandResult>
P4Switch::process(pfp::cp::DeleteCommand *cmd) {
  cout << "Delete Command at P4Switch" << endl;
  return cmd->success_result();
}

std::shared_ptr<pfp::cp::CommandResult>
P4Switch::process(pfp::cp::BootCompleteCommand*) {
  npulog(profile, cout << "Boot complete (now what?)" << endl;)
  return nullptr;
}

