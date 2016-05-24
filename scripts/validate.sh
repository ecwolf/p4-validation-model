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


thispath=$(dirname $(realpath $0))

if [[ -z "$2" ]]
then
  p4prog=$thispath/../test-data/simple_router.json
else
  p4prog=$(realpath $2)
fi

if [[ -z "$3" ]]
then
  table=$thispath/../test-data/table.txt
else
  table=$(realpath $3)
fi

if [[ -z "$4" ]]
then
  input=$thispath/../test-data/input.pcap
else
  input=$(realpath $4)
fi

if [[ ! -x "$1" ]] || [[ ! -f "$p4prog" ]] || [[ ! -f "$table" ]] || [[ ! -f "$input" ]]
then
  echo "Usage: $0 <model> <p4 program>.json <table population> <input>.pcap"
  exit 1
fi

model=$(realpath $1)

ref_dir=$(dirname $(realpath $0))/../p4switch/build/
reference=$ref_dir/p4switch-sim

if [[ ! -f $reference ]]
then
  echo -n "Building reference model... "
  cd $ref_dir
  cmake ../src >/dev/null 2>&1
  make >/dev/null 2>&1
  cp -r ../build_static/* ./ >/dev/null
  echo "done"
  cd -
fi

ref_out=$(mktemp)

echo -n "Executing reference model... "
cd $(dirname $reference)
$reference -Xp4 $p4prog -Xtpop $table -Xin $input -Xout $ref_out >/dev/null 2>&1
cd - >/dev/null 2>&1
echo "done"

model_out=$(mktemp)

echo -n "Executing model under test... "
cd $(dirname $model)
$model -Xp4 $p4prog -Xtpop $table -Xin $input -Xvalidation-out $model_out -v minimal # >/dev/null 2>&1
cd - >/dev/null 2>&1
echo "done"

echo "Comparing outputs:"
$(dirname $(realpath $0))/compare_output.sh $ref_out $model_out

#rm $ref_out $model_out
