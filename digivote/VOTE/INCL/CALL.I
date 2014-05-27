RUN tracerun("{&prg}").
RUN {&prg}(INPUT-OUTPUT prgSTAT{&param}).
RUN checkrun(INPUT-OUTPUT prgSTAT, INPUT "{&prg}").