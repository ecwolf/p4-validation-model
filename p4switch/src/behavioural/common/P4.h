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

#ifndef COMMON_P4_H_
#define COMMON_P4_H_

#include <string>
using std::string;
#include <map>
using std::map;

#include <bm_sim/switch.h>
#include <bm_sim/lookup_structures.h>
#include <iostream>
#include <cassert>

#include "ReadWriteLock.h"

extern int import_primitives();

struct Value {
  Value(bool match) : match{match} {}
  Value(bm::internal_handle_t handle) : handle{handle} {}
  bm::internal_handle_t handle = ~0;
  bool match                   = true;

  // Various ctors needed by trie types
  Value() {}
  Value(int) : match{false} {}
  Value(const Value & other) = default;
  Value(Value && other) = default;
  Value & operator=(const Value & other) = default;

  bool operator != (const Value & v) const{
    return match != v.match || handle != v.handle;
  }

  bool operator==(const Value& v) const {
    return match == v.match && handle == v.handle;
  }

  bool operator<(const Value& v) const {
    return handle < v.handle;
  }
};

/**
 * Multiton class that allows managing instances
 * of P4 switches. For now we only need a signleton
 * instance, but in the future we may want to have
 * many P4-programmed switches running in one
 * simulation.
 */
class P4 : public bm::Switch {
  private:

  private:
  static map<string, P4*> instances;

  public:
  static P4 * get(string name);

  private:
  // This class should only be constructed through the
  // static multiton method.
  P4();
  P4(P4&&)                 = default;
  P4(const P4&)            = default;
  P4& operator=(const P4&) = default;

  public:
  // Required but unused
  int  receive(int, const char *, int);
  void start_and_return();

  virtual ~P4();

  ReadWriteLock lock;
};

#endif
