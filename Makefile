##===- lib/Target/Mandarin/Makefile ------------------------*- Makefile -*-===##
#
#                     Vyacheslav Egorov
#
# This file is distributed under the MIT License
#
##===----------------------------------------------------------------------===##

LEVEL = ../../..
LIBRARYNAME = LLVMMandarinCodeGen
TARGET = Mandarin

# Make sure that tblgen is run, first thing.
BUILT_SOURCES = MandarinGenRegisterInfo.inc

DIRS = TargetInfo MCTargetDesc

include $(LEVEL)/Makefile.common

