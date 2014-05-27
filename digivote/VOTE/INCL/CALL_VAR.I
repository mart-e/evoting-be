RUN tracerun({&prg}).
RUN VALUE({&prg})(INPUT-OUTPUT prgSTAT{&param}).
RUN checkrun(INPUT-OUTPUT prgSTAT, INPUT {&prg}).