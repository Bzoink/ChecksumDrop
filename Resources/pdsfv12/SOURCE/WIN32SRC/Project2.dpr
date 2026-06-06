program Project2;

uses
  Forms,
  Unit1 in 'Unit1.pas' {mainform},
  Unit2 in 'Unit2.pas' {Form2};

{$R *.RES}

begin
  Application.Initialize;
  Application.Title := 'pdSFV v1.2';
  Application.CreateForm(Tmainform, mainform);
  Application.CreateForm(TForm2, Form2);
  Application.Run;
end.
