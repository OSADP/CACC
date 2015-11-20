#!/usr/bin/env python

#Copyright (c) 2013, Eduard Broecker 
#All rights reserved.
#
#Redistribution and use in source and binary forms, with or without modification, are permitted provided that
# the following conditions are met:
#
#    Redistributions of source code must retain the above copyright notice, this list of conditions and the
#    following disclaimer.
#    Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
#    following disclaimer in the documentation and/or other materials provided with the distribution.
#
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
#WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
#PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
#DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
#OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
#DAMAGE.

#
# this script imports sym-files in a canmatrix-object
# dbf-files are the can-matrix-definitions of the busmaster-project (http://rbei-etas.github.io/busmaster/)
#

from canmatrix import *
import re
import shlex

#TODO support for [START_PARAM_NODE_RX_SIG]
#TODO support for [START_PARAM_NODE_TX_MSG]

# dbfImportEncoding = 'iso-8859-1'

# def decodeDefine(line):
# 	(define, valueType, value) = line.split(',',2)			 
# 	valueType = valueType.strip()
# 	if valueType == "INT" or valueType == "HEX":
# 		(Min, Max, default) = value.split(',',2)
# 		myDef = valueType + ' ' + Min.strip() + ' ' + Max.strip()
# 		default = default.strip()
# 	elif valueType == "ENUM":
# 		(enums, default) = value.rsplit(',',1)
# 		myDef = valueType + "  " + enums[1:]
# 	elif valueType == "STRING":
# 		myDef = valueType 
# 		default = value
# 	else:
# 		print line

# 	return define[1:-1], myDef, default


def importSym(filename):

	db = CanMatrix()
	with open(filename,"r") as f:
		section = ''
		current_frame = Frame(0x000, None, 0, None)
		for line in f:
			line = line.strip()
			
			if section == '{ENUMS}':
				if line.startswith("{SENDRECEIVE}"):
					section = '{SENDRECEIVE}'
				else:
					# Do nothing for now
					continue

			if section == '{SENDRECEIVE}':
				if line.startswith("["): # found a new frame
					current_frame = Frame(0x000, None, 0, None)
					current_frame._name = line[1:-1]
				elif line.startswith("ID="):
					current_frame._Id = int(line.split("=")[1][:-1], 16)
				elif line.startswith("DLC="):
					current_frame._Size = int(line.split("=")[1])
					db._fl.addFrame(current_frame) # frame is fully built here
				elif line.startswith("Var="):
					signal_arr = shlex.split(line[4:]) #use shlex to keep quotes together
					# print signal_arr
					signal = Signal(signal_arr[0], signal_arr[2].split(",")[0], signal_arr[2].split(",")[1], 0, "+", 0, 0, 0, 0, "", ["Dummy_FO"])
					# signal._name = signal_arr[0]
					# signal._startbit = signal_arr[2].split(",'")[0]
					# signal._signalsize = signal_arr[2].split(",'")[1]
					if "-m" in signal_arr:
						signal._byteorder = 0
						sbit = signal._startbit
						
						signal._startbit = (sbit - (sbit%8)) + (7-(sbit % 8))
					else:
						signal._byteorder = 1
					if signal_arr[1] == "signed":
						signal._valuetype = "-"
					else:
						signal._valuetype = "+" #unsigned
					for attribute in signal_arr[2:]:
						if attribute.startswith("/u:"):
							signal._unit = attribute.split(":")[1]
						if attribute.startswith("/f:"):
							signal._factor = float(attribute.split(":")[1])
						if attribute.startswith("/o:"):
							signal._offset = float(attribute.split(":")[1])
						if attribute.startswith("/ln:"):
							signal._comment = attribute[4:]
						if attribute.startswith("/min:"):
							signal._min = float(attribute.split(":")[1])
						if attribute.startswith("/max:"):
							signal._max = float(attribute.split(":")[1])
					db._fl.addSignalToLastFrame(signal)

			if section == '':
				# if line.startswith("FormatVersion="):
				# 	db._attributes["sym_format_version"] = line.split("=")[1].split()[0]
				# 	# section = 'SignalDescription'

				# if line.startswith("Title="):
				# 	db._attributes["title"] = line.split("=")[1]
				# 	# section = 'FrameDescription'

				if line.startswith("{ENUMS}"):
					section = '{ENUMS}'

				if line.startswith("{SENDRECEIVE}"):
					section = '{SENDRECEIVE}'


	return db


