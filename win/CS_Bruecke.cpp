#include <vcl.h>
#pragma hdrstop
#pragma argsused

#include <tchar.h>
#include <stdio.h>

USEFORM("UCS_Dlg.cpp", Form1);

int _tmain(int argc, _TCHAR* argv[])
{
  try
  {
     Application->Initialize();
     Application->MainFormOnTaskBar = true;
     Application->CreateForm(__classid(TForm1), &Form1);
     Application->Run();
  }
  catch (Exception &exception)
  {
     Application->ShowException(&exception);
  }
  catch (...)
  {
     try
     {
       throw Exception("");
     }
     catch (Exception &exception)
     {
       Application->ShowException(&exception);
     }
  }
  return 0;
}
