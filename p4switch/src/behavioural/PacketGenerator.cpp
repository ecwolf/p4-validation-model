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

#include "./PacketGenerator.h"
#include <string>
#include <chrono>

#include "common/Packet.h"
#include "common/InputStimulus.h"
#include "common/PcapRepeater.h"

PacketGenerator::PacketGenerator(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):PacketGeneratorSIM(nm, parent, configfile) {  // NOLINT(whitespace/line_length)
  /*sc_spawn threads*/
  ThreadHandles.push_back(
        sc_spawn(sc_bind(&PacketGenerator::PacketGeneratorThread, this, 0)));
}

void PacketGenerator::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
void PacketGenerator::PacketGenerator_PortServiceThread() {
}

void PacketGenerator::PacketGeneratorThread(std::size_t thread_id) {
  std::string inputfile = SPARG("in");
  PcapRepeater pcapRepeater(inputfile);
  auto packet_gen = std::bind(&PcapRepeater::getNext, &pcapRepeater);

  auto seed =
       std::chrono::high_resolution_clock::now().time_since_epoch().count();
  std::mt19937 rng(seed);

  for (std::size_t i = 0; pcapRepeater.hasNext(); i++) {
    wait(0.5, SC_NS);

    Packet p(i, 1, -1, packet_gen());
    auto data_out = std::make_tuple(std::make_shared<Packet>(p), 1, 1);
    auto pkt = std::make_shared<InputStimulus>(i, data_out);
    out->put(pkt);
  }
}
