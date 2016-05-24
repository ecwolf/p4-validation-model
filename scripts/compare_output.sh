#!/usr/bin/env bash
#
# p4-validation-model: Reference model for P4 programs in the PFPSim Framework
#
# Copyright (C) 2016 Concordia Univ., Montreal
#     Samar Abdi
#     Umair Aftab
#     Gordon Bailey
#     Faras Dewal
#     Shafigh Parsazad
#     Eric Tremblay
#
# Copyright (C) 2016 Ericsson
#     Bochra Boughzala
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.
#


if [[ ! -f "$1" ]] || [[ ! -f "$2" ]]
then
  echo "Usage: $0 <pcap file> <pcap file>"
  exit 1
fi

output=$(mktemp)
expected=$(mktemp)

function cleanup_exit {
  rm $output $expected
  exit 1
}
trap cleanup_exit EXIT ERR

sudo tcpdump -t -n -vvv -x -e -r $1 > $output   2>/dev/null
sudo tcpdump -t -n -vvv -x -e -r $2 > $expected 2>/dev/null

if cmp --silent $output $expected
then
  (
  tput setf 2
  echo "$1 and $2 are Identical:"
  tput sgr0
  echo "== Diff of outputs =="
  echo -n "|                             OUTPUT                                                |"
  echo    "|                         EXPECTED OUTPUT                                           |"
  sdiff -d -w170 $expected $output
  echo
  echo
  ) | less -R
else
  (
  tput setf 1
  echo "$1 and $2 are different:"
  tput sgr0
  echo "== Diff of outputs =="
  echo -n "|                             OUTPUT                                                |"
  echo    "|                         EXPECTED OUTPUT                                           |"
  sdiff -d -w170 $expected $output
  echo
  echo

  #echo "== Side by side comparison of outputs =="
  #echo -n "|                           OUTPUT                                             |"
  #echo    "|                       EXPECTED OUTPUT                                        |"
  #sdiff -d -w160 $expected $output
  #echo
  #echo
  ) | less
fi
