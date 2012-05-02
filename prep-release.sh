#!/bin/bash
autoconf
rm -rf autom4te.cache
cd doc && make
