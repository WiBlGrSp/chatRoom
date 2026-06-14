#!/bin/bash
grep -rn \
--include="*.cpp" \
--include="*.h" \
"${1}" \
/home/twz/dev/chatRoom/