# The Etude Programming Language

---

Etude is lower-ish level programming language designed for educational
purposes but with the aspirations of writing real-world software.

Major features include HM-style global type inference.

---

Currently the compiler can run some basic programs but hopelessly lacking in
other ways. Sometimes in non-obvious ways. I do intend to fix all these
imperfections eventually, if the god is willing. Until then, have fun!

## Hosting

1. [Sourcehut](https://sr.ht/~orazov_ae/Etude/)
   - [Tickets for the compiler](https://todo.sr.ht/~orazov_ae/etude-compiler)
   - [Mailing list](https://lists.sr.ht/~orazov_ae/public-inbox)
   - [Source code](https://git.sr.ht/~orazov_ae/etude/refs)
2. [Other site](https://github.com/otakubeam/etude/)

## First steps

1. Look in the `examples/test` directory to get a feeling of the syntax
2. Build the compiler:
   - Dependencies: `catch2` and `fmt`.
   - Run `cmake` & `make`

     ``` sh
     cmake -B build
     cd build
     make -j8
     ```
   - Add the path to stdlib to your environment

     The author does this:
     ```
     export ETUDE_STDLIB="$HOME/.cache/etude"
     ln -s $ETUDE_STDLIB ./stdlib
     ```
   - Run the tests (if they fail, try different commit)
     ``` sh
     chmod +x test.sh
     ./test.sh
     ```
3. At this point you are ready to hack on the compiler


## Historic/Old

Check out the `0.2.0` tag of the repository: it contains the bytecode compiler
and interpreter as well as graphviz vizualizer.

