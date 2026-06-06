unit Unit2;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  StdCtrls;

type
  TForm2 = class(TForm)
    Memo1: TMemo;
    Button1: TButton;
    Button2: TButton;
    procedure Button2Click(Sender: TObject);
    procedure Button1Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Form2: TForm2;
  gresult: Integer;

implementation

{$R *.DFM}

procedure TForm2.Button2Click(Sender: TObject);
begin
 gresult := 0;
 Application.Terminate;
end;

procedure TForm2.Button1Click(Sender: TObject);
begin
 gresult := 1;
 Form2.Close;
end;

end.
