%include "elster_php.i" 
/*
%exception {
  try { 
    $action  } catch (const std::exception &e) 
  {    SWIG_exception_fail(SWIG_RuntimeError, e.what());  } 
}
*/%module elster_php%{/* Put headers and other declarations here */ 
#include "elster_php.h"%}%include "elster_php.h"