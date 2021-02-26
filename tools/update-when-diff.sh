#!/bin/bash
#
        tools/wrap-txt2c.sh $< $1.new
        if test -r $1; then                         \
          cmp $1.new $1 >/dev/null ||               \
            (mv -f $1.new $1;                       \
            echo '  wrap  '$(WRAP) $< to $(@F) ) \
        else                                        \
          mv $@.new $@; echo $(WRAP) $< to $(@F);   \
        fi
#
