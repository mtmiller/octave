prefer_zero_one_indexing = "true";
prefer_column_vectors = "false";
do_fortran_indexing = "false";
a = [9,8;7,6];
all (a(1,[1,1]) == [9,8])
