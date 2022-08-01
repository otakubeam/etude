# Types

[Type checker for C/C++]
(https://www.cse.chalmers.se/edu/year/2011/course/TIN321/laborations/lab2/lab2.html)

      typedef enum {
        OBJ_CLASS,
        ---------
        OBJ_CLOSURE,
        OBJ_FN,
        OBJ_INSTANCE,
        OBJ_LIST,
        ...
        OBJ_STRING,
        OBJ_UPVALUE
      } ObjType;

I wonder how it's better to get tagged unions to work?

Hare:

         struct context {
           struct modcache **modcache;
           struct define *defines;
           struct type_store *store;
           const struct type *fntype;
           struct identifier *ns;
           bool is_test;
           struct scope *unit;
           struct scope *scope;
           bool deferring;
           int id;
           struct errors *errors;
           struct errors **next;
         v};

