%include "elster_perl.i" 
%exception {
  try { 
    $action  } catch (const std::exception &e) 
  {    SWIG_exception_fail(SWIG_RuntimeError, e.what());  } 
}
%module elster_perl%{/* Put headers and other declarations here */ 
#include "elster_perl.h"%}%include "elster_perl.h"