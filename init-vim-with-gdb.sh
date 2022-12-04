#!/bin/bash
set -xue

mkdir -p /tmp/install && cd /tmp/install

# install dev env
if [[ ! -e vim-7.4.tar.bz2 ]]; then
    wget ftp://ftp.vim.org/pub/vim/unix/vim-7.4.tar.bz2
fi

if [[ ! -e vimgdb-for-vim74.zip ]]; then
    wget https://github.com/larrupingpig/vimgdb-for-vim7.4/archive/master.zip -O vimgdb-for-vim74.zip
fi

rm -rf /tmp/vim*
tar xjvf vim-7.4.tar.bz2 -C /tmp
unzip vimgdb-for-vim74.zip -d /tmp
cd /tmp && patch -p0 < vimgdb-for-vim7.4-master/vim74.patch

cd vim74/src
./configure \
    --enable-gdb \
    --enable-multibyte \
    --enable-fontset --enable-xim  \
    --enable-gui=auto \
    --enable-rubyinterp=dynamic \
    --enable-rubyinterp --enable-cscope \
    --enable-sniff \
    --enable-luainterp=dynamic \
    --with-x --with-features=huge

make CFLAGS="-O2 -D_FORTIFY_SOURCE=1"
make CFLAGS="-O2 -D_FORTIFY_SOURCE=1" install

ln -sf /opt/bin/vim /usr/bin/vi

# install amix vimrc
if [[ ! -e ~/.vim_runtime ]]; then
    git clone --depth=1 https://github.com/amix/vimrc.git ~/.vim_runtime
    sh ~/.vim_runtime/install_awesome_vimrc.sh

    cp -r /tmp/vimgdb-for-vim7.4-master/vimgdb_runtime/ /root/.vim_runtime/sources_non_forked/
    echo run macros/gdb_mappings.vim >> ~/.vim_runtime/my_configs.vim

    git clone https://github.com/ervandew/supertab /root/.vim_runtime/sources_non_forked/supertab
    git clone https://github.com/vim-scripts/taglist.vim /root/.vim_runtime/sources_non_forked/taglist
    git clone https://github.com/vim-scripts/a.vim.git /root/.vim_runtime/sources_non_forked/a
fi

cd -
