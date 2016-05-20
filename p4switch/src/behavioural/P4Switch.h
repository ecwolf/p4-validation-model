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

#ifndef BEHAVIOURAL_P4SWITCH_H_
#define BEHAVIOURAL_P4SWITCH_H_
#include <string>
#include <vector>
#include "../structural/P4SwitchSIM.h"

class P4Switch: public P4SwitchSIM, public pfp::cp::CommandProcessor {
 public:
  SC_HAS_PROCESS(P4Switch);
  /*Constructor*/
  explicit P4Switch(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~P4Switch() = default;

 public:
  void init();

  std::shared_ptr<pfp::cp::CommandResult>
    send_command(const std::shared_ptr<pfp::cp::Command> & cmd) override;

  std::shared_ptr<pfp::cp::CommandResult>
      process(pfp::cp::InsertCommand*) override;
  std::shared_ptr<pfp::cp::CommandResult>
      process(pfp::cp::ModifyCommand*) override;
  std::shared_ptr<pfp::cp::CommandResult>
      process(pfp::cp::DeleteCommand*) override;
  std::shared_ptr<pfp::cp::CommandResult>
      process(pfp::cp::BootCompleteCommand*) override;

  void command_processing_thread();
 private:
  sc_event new_command;
  std::shared_ptr<pfp::cp::Command> current_command;

  sc_event command_processed;
  std::shared_ptr<pfp::cp::CommandResult> current_result;

 private:
  void P4Switch_PortServiceThread();
  void P4SwitchThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;
};

#endif  // BEHAVIOURAL_P4SWITCH_H_
