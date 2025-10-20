#
# MIT License
#
# Copyright (c) 2023-24 AESilky
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# ############################################################################
#
# Makefile to create:
#  * The manuals (pdf from the adoc)
#
AWK				?= awk
GREP			?= grep
KILL			?= kill
KILL_FLAGS		?= -f
PDFGEN			?= asciidoctor-pdf
PR				?= pr
PS				?= ps
PS_FLAGS		?= -W
PS_FIELDS		?= "9 47 100"
SHELL			:= /bin/bash
SORT			?= sort

DOC_DIR			?= .
IMAGES_DIR		?= $(DOC_DIR)/images

MANUAL_SETTINGS	:= $(DOC_DIR)/man_settings.adoc
MuKOB_THEME		:= $(DOC_DIR)/MuKOB-theme.yml
MK_KS_IF_REFMAN	:= $(DOC_DIR)/Ref-Manual-KSIF-MkV.pdf
MK_KS_IF_REFMAN_IMAGES	:= $(IMAGES_DIR)/*.png $(IMAGES_DIR)/*.jpg

vpath %.py		src.py
vpath %.pyw		src.py

# macros
#
# $(call kill-program,awk-pattern)
define kill-program
	@ $(PS) $(PS_FLAGS) |										\
	$(AWK) 'BEGIN	{ FIELDWIDTHS = $(PS_FIELDS) }				\
		/$1/	{												\
						print "Killing " $$3;					\
						system( "$(KILL) $(KILL_FLAGS) " $$1 )	\
					}'
endef

# Default - Make all of the documentation
.PHONY: all
all: docs

.PHONY: docs
docs: $(MK_KS_IF_REFMAN)

$(MK_KS_IF_REFMAN): $(MANUAL_SETTINGS) $(MuKOB_THEME) $(MK_KS_IF_REFMAN_IMAGES)

# help - Print a list of all targets in this makefile
.PHONY: help
help:
	@$(MAKE_COMMAND) --print-data-base --question no-such-target | \
	$(GREP) -v -e '^no-such-target' -e '^makefile' | \
	$(AWK) '/^[^.%][-A-Za-z0-9_]*:/ { print substr($$1, 1, length($$1)-1) }' | \
	$(SORT) | \
	$(PR) --omit-pagination --width=80 --columns=4

%.pdf: %.adoc
	$(PDFGEN) $<

Ref-Manual-KSIF-MkIII.pdf: Ref-Manual-KSIF-MkIII.adoc $(MANUAL_SETTINGS) $(MuKOB_THEME) $(MK_KS_IF_REFMAN_IMAGES)
