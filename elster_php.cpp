/*
 *
 *  Copyright (C) 2017 Jürg Müller, CH-5524
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see http://www.gnu.org/licenses/ .
 */

/*

 php-Code (siehe "mac/Test.php"):
 
   dl("elster_php.dylib"); # muss im richtigen Verzeichnis sein /usr/lib/...
   include("elster_php.php");
 
 
   $v = "000a";
   echo "name: " .elster::getname($v) . "  type: " . elster::gettype($v) . "\n";

 "dylib" ist für den MAC. In Windows steht an dieser Stelle "dll" und in Linux "so".
 
*/

#include "elster_php.h"

#include "elster_swig.cpp"
