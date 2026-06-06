unit Unit1;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  ExtCtrls, StdCtrls, Math, jpeg, Gauges, FileCtrl, FoolTrayIcon, Registry;

type
  Tmainform = class(TForm)
    IntroPanel: TPanel;
    CommandPanel: TPanel;
    cmdLabel1: TLabel;
    cmdLabel3: TLabel;
    cmdLabel2: TLabel;
    IntroImage: TImage;
    IntroImage2: TImage;
    IsonewsBanner: TImage;
    SFV_Create_Panel: TPanel;
    Status1: TMemo;
    SFVname1: TEdit;
    SFV_FN_But1: TButton;
    BannerName: TEdit;
    BannerButton: TButton;
    Gauge1: TGauge;
    AddFS_Chk: TCheckBox;
    WINSFV_Chk: TCheckBox;
    ListBox1: TListBox;
    ClearList1: TButton;
    AddAll1: TButton;
    Label2: TLabel;
    DriveComboBox1: TDriveComboBox;
    DirectoryListBox1: TDirectoryListBox;
    FileListBox1: TFileListBox;
    SFVcreateHintImage: TImage;
    CreateSFVnow: TButton;
    SaveDialog1: TSaveDialog;
    OpenDialog1: TOpenDialog;
    AbortButton1: TButton;
    TrayIcon: TFoolTrayIcon;
    StartUpTimer: TTimer;
    Check_All_Panel: TPanel;
    Image1: TImage;
    ChA_SFV_Select: TButton;
    ChA_SFV_FN_Edit: TEdit;
    OpenDialog2: TOpenDialog;
    ChA_CheckNow: TButton;
    Status2: TMemo;
    Gauge2: TGauge;
    Status2b: TMemo;
    Check_Single_Panel: TPanel;
    Image2: TImage;
    SingleSFVFileSelect: TButton;
    SingleSFVName: TEdit;
    OpenDialog3: TOpenDialog;
    DriveComboBox2: TDriveComboBox;
    DirectoryListBox2: TDirectoryListBox;
    FileListBox2: TFileListBox;
    ListBox2: TListBox;
    Status3: TMemo;
    SingleCheckNow: TButton;
    Label1: TLabel;
    SingleAddAll: TButton;
    SingleClearList: TButton;
    Gauge3: TGauge;
    procedure cmdLabel1MouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure CommandPanelMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure cmdLabel2MouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure cmdLabel3MouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure IntroImageMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure IntroImage2MouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure IsonewsBannerClick(Sender: TObject);
    procedure IsonewsBannerMouseMove(Sender: TObject; Shift: TShiftState;
      X, Y: Integer);
    procedure cmdLabel1Click(Sender: TObject);
    procedure SFV_Create_PanelMouseMove(Sender: TObject;
      Shift: TShiftState; X, Y: Integer);
    procedure Status1MouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure SFV_FN_But1Click(Sender: TObject);
    procedure SFVname1Exit(Sender: TObject);
    procedure BannerButtonClick(Sender: TObject);
    procedure BannerNameExit(Sender: TObject);
    procedure AddFS_ChkClick(Sender: TObject);
    procedure WINSFV_ChkClick(Sender: TObject);
    procedure FileListBox1DblClick(Sender: TObject);
    procedure ClearList1Click(Sender: TObject);
    procedure ListBox1DblClick(Sender: TObject);
    procedure AddAll1Click(Sender: TObject);
    procedure CreateSFVnowClick(Sender: TObject);
    procedure AbortButton1Click(Sender: TObject);
    procedure SFVcreateHintImageMouseMove(Sender: TObject;
      Shift: TShiftState; X, Y: Integer);
    procedure ListBox1MouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure DirectoryListBox1MouseMove(Sender: TObject;
      Shift: TShiftState; X, Y: Integer);
    procedure TrayIconClick(Sender: TObject);
    procedure StartUpTimerTimer(Sender: TObject);
    procedure cmdLabel2Click(Sender: TObject);
    procedure ChA_SFV_SelectClick(Sender: TObject);
    procedure ChA_CheckNowClick(Sender: TObject);
    procedure Image1MouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure Status2bMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure Status2MouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure Check_All_PanelMouseMove(Sender: TObject; Shift: TShiftState;
      X, Y: Integer);
    procedure cmdLabel3Click(Sender: TObject);
    procedure SingleSFVFileSelectClick(Sender: TObject);
    procedure FileListBox2DblClick(Sender: TObject);
    procedure SingleAddAllClick(Sender: TObject);
    procedure ListBox2DblClick(Sender: TObject);
    procedure SingleClearListClick(Sender: TObject);
    procedure SingleCheckNowClick(Sender: TObject);
    procedure Image2MouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure Status3MouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure Image2MouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure ListBox2MouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure DirectoryListBox2MouseMove(Sender: TObject;
      Shift: TShiftState; X, Y: Integer);
    procedure FileListBox2MouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  mainform: Tmainform;
  abort_current_operation: Integer;

implementation

uses Unit2;

{$R *.DFM}

procedure LowLightCommandButtons;
begin
 if mainform.cmdLabel1.Font.Color <> clWhite then mainform.cmdLabel1.Font.Color := clWhite;
 if mainform.cmdLabel2.Font.Color <> clWhite then mainform.cmdLabel2.Font.Color := clWhite;
 if mainform.cmdLabel3.Font.Color <> clWhite then mainform.cmdLabel3.Font.Color := clWhite;
end;

procedure Tmainform.cmdLabel1MouseMove(Sender: TObject; Shift: TShiftState;
  X, Y: Integer);
var distance, color_offset: Integer;
begin
 // Lowlight other buttons
 if cmdLabel2.Font.Color <> clWhite then cmdLabel2.Font.Color := clWhite;
 if cmdLabel3.Font.Color <> clWhite then cmdLabel3.Font.Color := clWhite;
 // Calculate cursor distance from center
 distance :=  round( sqrt( abs( Power ( (cmdLabel1.Height/2) - y ,2) +
              (cmdLabel1.Width div cmdLabel1.Height) *
              Power ( (cmdLabel1.Width /2) - x ,2) )));
 // Calculate new color
 color_offset := 255 - distance;
 if color_offset < 0 then color_offset := 0;
 // Set new color if needed
 if abs (cmdLabel1.Font.Color - ($00FFFFFF - color_offset)) > 50 then
    cmdLabel1.Font.Color := $00FFFFFF - color_offset;
end;

procedure Tmainform.CommandPanelMouseMove(Sender: TObject;
  Shift: TShiftState; X, Y: Integer);
begin
 LowLightCommandButtons;
end;

procedure Tmainform.cmdLabel2MouseMove(Sender: TObject; Shift: TShiftState;
  X, Y: Integer);
var distance, color_offset: Integer;
begin
 // Lowlight other buttons
 if cmdLabel1.Font.Color <> clWhite then cmdLabel1.Font.Color := clWhite;
 if cmdLabel3.Font.Color <> clWhite then cmdLabel3.Font.Color := clWhite;
 // Calculate cursor distance from center
 distance :=  round( sqrt( abs( Power ( (cmdLabel2.Height/2) - y ,2) +
              30/(cmdLabel2.Width div cmdLabel2.Height) *
              Power ( (cmdLabel2.Width /2) - x ,2) )));
 // Calculate new color
 color_offset := 255 - distance;
 if color_offset < 0 then color_offset := 0;
 // Set new color if needed
 if abs (cmdLabel2.Font.Color - ($00FFFFFF - color_offset)) > 50 then
    cmdLabel2.Font.Color := $00FFFFFF - color_offset;
end;

procedure Tmainform.cmdLabel3MouseMove(Sender: TObject; Shift: TShiftState;
  X, Y: Integer);
var distance, color_offset: Integer;
begin
 // Lowlight other buttons
 if cmdLabel1.Font.Color <> clWhite then cmdLabel1.Font.Color := clWhite;
 if cmdLabel2.Font.Color <> clWhite then cmdLabel2.Font.Color := clWhite;
 // Calculate cursor distance from center
 distance :=  round( sqrt( abs( Power ( (cmdLabel3.Height/2) - y ,2) +
              30/(cmdLabel3.Width div cmdLabel3.Height) *
              Power ( (cmdLabel3.Width /2) - x ,2) )));
 // Calculate new color
 color_offset := 255 - distance;
 if color_offset < 0 then color_offset := 0;
 // Set new color if needed
 if abs (cmdLabel3.Font.Color - ($00FFFFFF - color_offset)) > 50 then
    cmdLabel3.Font.Color := $00FFFFFF - color_offset;
end;

procedure Tmainform.IntroImageMouseMove(Sender: TObject;
  Shift: TShiftState; X, Y: Integer);
begin
 LowLightCommandButtons;
end;

procedure Tmainform.IntroImage2MouseMove(Sender: TObject;
  Shift: TShiftState; X, Y: Integer);
begin
 LowLightCommandButtons;
end;

procedure Tmainform.IsonewsBannerClick(Sender: TObject);
begin
 // Direct default-Webbrower to Isonews's website
 // First command does nothing on Windows NT
 // Second command does nothing on Windows 9x ;-)
  WinExec(PChar('command.com /c start http://pdsfv.isonews.com'), SW_HIDE);
  WinExec(PChar('cmd.exe /c start http://pdsfv.isonews.com'), SW_HIDE);
end;

procedure Tmainform.IsonewsBannerMouseMove(Sender: TObject;
  Shift: TShiftState; X, Y: Integer);
begin
 LowLightCommandButtons;
end;

procedure Tmainform.cmdLabel1Click(Sender: TObject);
begin
//  MessageBeep (MB_OK);
  Check_All_Panel.Visible := False;
  Check_Single_Panel.Visible := False;  
  SFV_Create_Panel.Visible := True;
  SFV_Create_Panel.BringToFront;
end;

procedure Tmainform.SFV_Create_PanelMouseMove(Sender: TObject;
  Shift: TShiftState; X, Y: Integer);
begin
 LowLightCommandButtons;
end;

procedure Tmainform.Status1MouseMove(Sender: TObject; Shift: TShiftState;
  X, Y: Integer);
begin
 LowLightCommandButtons;
end;

procedure Tmainform.SFV_FN_But1Click(Sender: TObject);
begin
 if SaveDialog1.Execute then
 begin
   SFVname1.Text := SaveDialog1.FileName;
   SFVname1.Text := LowerCase (SFVname1.Text);
   Status1.Lines.Add('Output filename -> ' + SFVname1.Text);
 end;
end;

procedure Tmainform.SFVname1Exit(Sender: TObject);
begin
 if SFVname1.Text <> '' then
 begin
   if Pos('.', SFVname1.Text) = 0 then
      SFVname1.Text := SFVname1.Text + '.sfv';
   SFVname1.Text := LowerCase(SFVname1.Text);
   Status1.Lines.Add('Output filename -> ' + SFVname1.Text);
 end;
end;

procedure Tmainform.BannerButtonClick(Sender: TObject);
begin
 if OpenDialog1.Execute then
 begin
  BannerName.Text := OpenDialog1.FileName;
  Status1.Lines.Add ('Bannerfile -> ' + BannerName.Text);
 end;
end;

procedure Tmainform.BannerNameExit(Sender: TObject);
begin
 if BannerName.Text <> '' then
  Status1.Lines.Add ('Bannerfile -> ' + BannerName.Text);
end;

procedure Tmainform.AddFS_ChkClick(Sender: TObject);
begin
 if AddFS_Chk.Checked then
  Status1.Lines.Add ('Adding filesizes as comment -> enabled')
 else
  Status1.Lines.Add ('Adding filesizes as comment -> disabled');
end;

procedure Tmainform.WINSFV_ChkClick(Sender: TObject);
var answer, asked: Longint;
begin
 asked := 0;
 if (WINSFV_Chk.Checked = True) then
  begin
   asked := 1;
   answer := Application.MessageBox ('WIN-SFV compatibility mode implies restrictions to the output file format and is usually not needed anymore. Are you sure that you want to enable WIN-SFV compatibility?', 'WIN-SFV compatibility?', MB_YESNO+MB_DEFBUTTON2+MB_ICONQUESTION);
   if (answer = 6) then WINSFV_Chk.Checked := True
                   else WINSFV_Chk.Checked := False;
  end;
 if (WINSFV_Chk.Checked = True) then
  Status1.Lines.Add('WIN-SFV compatibility -> on')
 else
  if (asked <> 1) then
   Status1.Lines.Add('WIN-SFV compatibility -> off');
end;

procedure Tmainform.FileListBox1DblClick(Sender: TObject);
var answer: longint;
begin
 if Pos('.0', FileListBox1.FileName) = 0 then    // RAR-Archive .001 - .0?? !?
  if Pos('.R', UpperCase(FileListBox1.FileName)) = 0 then // RAR-Archive .R?? !?
   if Pos('.M', UpperCase(FileListBox1.FileName)) = 0 then // MP3 File !?
    begin
     answer := Application.MessageBox ('Adding files not part of a RAR archive or MP3 set to the SFV file is generally a bad idea. Sure you want to add this file?', PChar(ExtractFileName(FileListBox1.FileName)), MB_YESNO+MB_DEFBUTTON2+MB_ICONQUESTION);
     if answer <> 6 then Exit;
    end;
 if Length (ExtractFileName(FileListBox1.FileName)) > 64 then
  if Pos ('.MP3', UpperCase(FileListBox1.FileName)) <> 0 then
  begin
     answer := Application.MessageBox ('MP3 filenames usually should be no more than 64 characters in length. Sure you want to add this file?', PChar(ExtractFileName(FileListBox1.FileName)), MB_YESNO+MB_DEFBUTTON2+MB_ICONQUESTION);
     if answer <> 6 then Exit;
  end;
 ListBox1.Items.Add(FileListBox1.FileName);
// if (Length (FileListBox1.FileName) > 80) then ListBox1.
end;

procedure Tmainform.ClearList1Click(Sender: TObject);
begin
 ListBox1.Items.Clear;
end;

procedure Tmainform.ListBox1DblClick(Sender: TObject);
begin
 ListBox1.Items.Delete(ListBox1.ItemIndex);
end;

procedure Tmainform.AddAll1Click(Sender: TObject);
var cnt: Integer;
begin
 for cnt := 0 to (FileListBox1.Items.Count - 1) do
  begin
   FileListBox1.ItemIndex := cnt;
   FileListBox1DblClick(self);
  end;
end;

function UpdateCRC (CRC : LongWord; Buffer : array of Char; count : LongWord) : LongWord;
var bpos: Integer;
    tmp1, tmp2, tmp3: LongWord;
const CRCTABLE : array [0..255] of  LongWord =
  (
    $00000000, $77073096, $ee0e612c, $990951ba, $076dc419, $706af48f, $e963a535, $9e6495a3
  , $0edb8832, $79dcb8a4, $e0d5e91e, $97d2d988, $09b64c2b, $7eb17cbd, $e7b82d07, $90bf1d91
  , $1db71064, $6ab020f2, $f3b97148, $84be41de, $1adad47d, $6ddde4eb, $f4d4b551, $83d385c7
  , $136c9856, $646ba8c0, $fd62f97a, $8a65c9ec, $14015c4f, $63066cd9, $fa0f3d63, $8d080df5
  , $3b6e20c8, $4c69105e, $d56041e4, $a2677172, $3c03e4d1, $4b04d447, $d20d85fd, $a50ab56b
  , $35b5a8fa, $42b2986c, $dbbbc9d6, $acbcf940, $32d86ce3, $45df5c75, $dcd60dcf, $abd13d59
  , $26d930ac, $51de003a, $c8d75180, $bfd06116, $21b4f4b5, $56b3c423, $cfba9599, $b8bda50f
  , $2802b89e, $5f058808, $c60cd9b2, $b10be924, $2f6f7c87, $58684c11, $c1611dab, $b6662d3d
  , $76dc4190, $01db7106, $98d220bc, $efd5102a, $71b18589, $06b6b51f, $9fbfe4a5, $e8b8d433
  , $7807c9a2, $0f00f934, $9609a88e, $e10e9818, $7f6a0dbb, $086d3d2d, $91646c97, $e6635c01
  , $6b6b51f4, $1c6c6162, $856530d8, $f262004e, $6c0695ed, $1b01a57b, $8208f4c1, $f50fc457
  , $65b0d9c6, $12b7e950, $8bbeb8ea, $fcb9887c, $62dd1ddf, $15da2d49, $8cd37cf3, $fbd44c65
  , $4db26158, $3ab551ce, $a3bc0074, $d4bb30e2, $4adfa541, $3dd895d7, $a4d1c46d, $d3d6f4fb
  , $4369e96a, $346ed9fc, $ad678846, $da60b8d0, $44042d73, $33031de5, $aa0a4c5f, $dd0d7cc9
  , $5005713c, $270241aa, $be0b1010, $c90c2086, $5768b525, $206f85b3, $b966d409, $ce61e49f
  , $5edef90e, $29d9c998, $b0d09822, $c7d7a8b4, $59b33d17, $2eb40d81, $b7bd5c3b, $c0ba6cad
  , $edb88320, $9abfb3b6, $03b6e20c, $74b1d29a, $ead54739, $9dd277af, $04db2615, $73dc1683
  , $e3630b12, $94643b84, $0d6d6a3e, $7a6a5aa8, $e40ecf0b, $9309ff9d, $0a00ae27, $7d079eb1
  , $f00f9344, $8708a3d2, $1e01f268, $6906c2fe, $f762575d, $806567cb, $196c3671, $6e6b06e7
  , $fed41b76, $89d32be0, $10da7a5a, $67dd4acc, $f9b9df6f, $8ebeeff9, $17b7be43, $60b08ed5
  , $d6d6a3e8, $a1d1937e, $38d8c2c4, $4fdff252, $d1bb67f1, $a6bc5767, $3fb506dd, $48b2364b
  , $d80d2bda, $af0a1b4c, $36034af6, $41047a60, $df60efc3, $a867df55, $316e8eef, $4669be79
  , $cb61b38c, $bc66831a, $256fd2a0, $5268e236, $cc0c7795, $bb0b4703, $220216b9, $5505262f
  , $c5ba3bbe, $b2bd0b28, $2bb45a92, $5cb36a04, $c2d7ffa7, $b5d0cf31, $2cd99e8b, $5bdeae1d
  , $9b64c2b0, $ec63f226, $756aa39c, $026d930a, $9c0906a9, $eb0e363f, $72076785, $05005713
  , $95bf4a82, $e2b87a14, $7bb12bae, $0cb61b38, $92d28e9b, $e5d5be0d, $7cdcefb7, $0bdbdf21
  , $86d3d2d4, $f1d4e242, $68ddb3f8, $1fda836e, $81be16cd, $f6b9265b, $6fb077e1, $18b74777
  , $88085ae6, $ff0f6a70, $66063bca, $11010b5c, $8f659eff, $f862ae69, $616bffd3, $166ccf45
  , $a00ae278, $d70dd2ee, $4e048354, $3903b3c2, $a7672661, $d06016f7, $4969474d, $3e6e77db
  , $aed16a4a, $d9d65adc, $40df0b66, $37d83bf0, $a9bcae53, $debb9ec5, $47b2cf7f, $30b5ffe9
  , $bdbdf21c, $cabac28a, $53b39330, $24b4a3a6, $bad03605, $cdd70693, $54de5729, $23d967bf
  , $b3667a2e, $c4614ab8, $5d681b02, $2a6f2b94, $b40bbe37, $c30c8ea1, $5a05df1b, $2d02ef8d
  );
begin
 bpos := 0;
 if ( count <> 0 ) then
 begin
  while (count > 0) do
   begin
    tmp1 := ((CRC shr 8) and $FFFFFF);
    tmp2 := (CRC and $FF) xor Byte(buffer[bpos]);
    tmp3 := tmp1 xor CRCTABLE[tmp2];
    CRC := tmp3;
    inc (bpos);
    dec (count);
   end;
 end;
 UpdateCRC := CRC;
end;

function GetFileCRC (FileName: String; Gauge: Integer) : LongInt;
var crc : LongWord; totalread, localread : LongInt;
    f : File; buffer : array[0..65535] of Char;
    cpusaver: Integer; fs: longint;
begin
 crc := $FFFFFFFF;
 if FileExists ( FileName ) then
 begin
  AssignFile (f, FileName);
  Reset (f, 1);
  fs := FileSize(f);
  totalread := 0;
  repeat
   BlockRead(f, buffer, SizeOf(buffer), localread);
   for cpusaver := 1 to 5 do Application.ProcessMessages();
   crc := UpdateCRC (crc, buffer, localread);
   totalread := totalread + localread;
   if Gauge = 1 then mainform.Gauge1.Progress := round (100 * (totalread / fs));
   if Gauge = 2 then mainform.Gauge2.Progress := round (100 * (totalread / fs));
   if Gauge = 3 then mainform.Gauge3.Progress := round (100 * (totalread / fs));
   for cpusaver := 1 to 5 do Application.ProcessMessages();
  until (localread = 0);
  CloseFile(f);
  crc := crc xor $FFFFFFFF;
 end;
GetFileCRC := crc;
end;


procedure Tmainform.CreateSFVnowClick(Sender: TObject);
var output, Banner: TextFile;
    cur_line: String;
    cnt: Integer;
    sr: TSearchRec;
    fdate: TDateTime;
    crazyDate, crazyTime: String[50];
    MyCRC: LongWord;
begin
// here we create the SFV file
  if SFVname1.Text = '' then
  begin
    MessageBox(0, 'You must specify a filename for the SFV file to be created!', 'Looks like you need a hint! :)', MB_OK);
    Exit
  end;

  if ListBox1.Items.Count = 0 then
  begin
    MessageBox(0, 'You should select at least one file to be added to the SFV!', 'Looks like you need a hint! :)', MB_OK);
    Exit
  end;

  Status1.Lines.Add ('Creating ' + SFVname1.Text);
  Application.ProcessMessages();

  AssignFile (output, SFVname1.Text);
  Rewrite (output);

  // create Generated-by line
  cur_line := '; Generated by ';
  if (WINSFV_Chk.Checked) then
   cur_line := cur_line + 'WIN-SFV32 v1 (pdSFV v1.2)'
  else cur_line := cur_line + 'pdSFV v1.2 (WIN)';

  crazyTime := FormatDateTime('hh":"nn"."ss', Time);
  crazyDate := FormatDateTime('yyyy"-"mm"-"dd', Date);

  cur_line := cur_line + ' on ' + crazyDate + ' at ' + crazyTime;
  writeln (output, cur_line);
  writeln (output, ';');

  // add banner
  if (BannerName.Text <> '') then
  begin
   if (FileExists (BannerName.Text)) then
   begin
    AssignFile(Banner, BannerName.Text);
    Reset(Banner);
    writeln(output, ';!SFV_COMMENT_START');  writeln(output, ';');
    while not eof(Banner) do
     begin
      readln(Banner, cur_line);
      writeln(output, '; ' + cur_line);
     end;
    CloseFile(Banner);
    writeln(output, ';!SFV_COMMENT_END');  writeln(output, ';');
    Status1.Lines.Add('(Bannerfile added) ...');
   end else begin
    Application.MessageBox('The file you specified as banner was not found. No banner will be added to the SFV file.', 'Problem', MB_OK);
   end;
  end;

  if AddFS_Chk.Checked then
  begin
   for cnt := 0 to ListBox1.Items.Count - 1 do
   begin
    if FileExists(ListBox1.Items[cnt]) then
    begin
     FindFirst(ListBox1.Items[cnt], faAnyFile, sr);
     fdate := FileDateToDateTime(sr.Time);
     crazyTime := FormatDateTime('hh":"nn"."ss', fdate);
     crazyDate := FormatDateTime('yyyy"-"mm"-"dd', fdate);
     writeln(output, Format('; %12s  %s %s %s',[IntToStr(sr.Size),crazyTime,crazyDate,ExtractFileName(ListBox1.Items[cnt])]) );
    end;
   end;
   Status1.Lines.Add ('(Added file size and date information) ...');
  end;

  writeln(output, ';');

  abort_current_operation := 0;
  CreateSFVnow.Visible := False;
  AbortButton1.Visible := True;
  Application.ProcessMessages;

  cnt := 0;
  while cnt <= ListBox1.Items.Count - 1 do
  begin
   ListBox1.Itemindex := cnt;
   Status1.Lines.Add ('Processing file ' + ExtractFileName(ListBox1.Items[ListBox1.Itemindex]) + ' ...');
   Application.ProcessMessages();
   MyCRC := GetFileCRC(ListBox1.Items[ListBox1.ItemIndex], 1);
   cur_line := Format('%s %08.08X', [ExtractFileName(ListBox1.Items[ListBox1.Itemindex]), MyCRC]);
   writeln (output, cur_line);
   Status1.Lines.Delete(Status1.Lines.Count - 1);
   Status1.Lines.Add ('('+ExtractFileName(ListBox1.Items[ListBox1.Itemindex])+' CRC = 0x'+
              Format('%08.08X', [MyCRC]) + ') ...');
   Application.ProcessMessages();
   if (abort_current_operation = 1) then
   begin
    cnt := Listbox1.items.count - 1;
    Status1.Lines.Add('[ Operation aborted by user ]');
   end;
   inc(cnt);
  end;

  CloseFile (output);
  if abort_current_operation = 0 then
  begin
   Status1.Lines.Add ('[ Finished. Ready... ]');
   Application.MessageBox('Your SFV file has been created successfully!', 'Progress information', MB_OK);
  end;
end;

procedure Tmainform.AbortButton1Click(Sender: TObject);
begin
 abort_current_operation := 1;
 Application.ProcessMessages;
 AbortButton1.Visible := False;
 CreateSFVnow.Visible := True;
 Application.ProcessMessages;
end;

procedure Tmainform.SFVcreateHintImageMouseMove(Sender: TObject;
  Shift: TShiftState; X, Y: Integer);
begin
 LowLightCommandButtons;
end;

procedure Tmainform.ListBox1MouseMove(Sender: TObject; Shift: TShiftState;
  X, Y: Integer);
begin
 LowLightCommandButtons;
end;

procedure Tmainform.DirectoryListBox1MouseMove(Sender: TObject;
  Shift: TShiftState; X, Y: Integer);
begin
 LowLightCommandButtons;
end;

procedure Tmainform.TrayIconClick(Sender: TObject);
begin
 TrayIcon.ShowMainForm;
end;

procedure Tmainform.StartUpTimerTimer(Sender: TObject);
var R: TRegIniFile;
begin
 StartUpTimer.Enabled := False; // This event only runs once after start
 if ParamCount > 0 then
 begin

   if LowerCase(ParamStr(1)) = '/reg' then
   begin
    R := TRegIniFile.Create('');
    with R do
    begin
     RootKey := HKEY_CLASSES_ROOT;
     WriteString ('.sfv', '', 'SFV');
     WriteString ('SFV', '', 'pdSFV CRC-32 datafile');
     WriteString ('SFV\DefaultIcon', '', ParamStr(0)+',0');
     WriteString ('SFV\Shell', '', 'SFV-Check');
//     WriteString ('SFV\Shell\First_Action', '', 'View');
//     WriteString ('SFV\Shell\First_Action\command', '', ParamStr(0)+' %1');
     WriteString ('SFV\Shell\Open', '', 'Open');
     WriteString ('SFV\Shell\Open\command', '', ParamStr(0)+' %1');
     WriteString ('SFV\Shell\SFV-Check', '', 'SFV-Check');
     WriteString ('SFV\Shell\SFV-Check\command', '', ParamStr(0) + ' %1');
     Free;
    end;
    MessageBox(0, 'pdSFV has been associated with SFV files!', 'Windows Registry updated', mb_ok);
    Exit;
   end; // associate .SFV extension with us

   if FileExists(ParamStr(1)) then
   begin
     Form2.Caption := ParamStr(1);
     Form2.Memo1.Lines.LoadFromFile (ParamStr(1));
     Form2.ShowModal;
     if gresult = 1 then
     begin
      cmdLabel2Click (self);
      ChA_SFV_FN_Edit.Text := ParamStr(1);
      ChA_CheckNowClick(self);
     end;
   end; // Run SFV Check on file given on command line

 end;
end;

procedure Tmainform.cmdLabel2Click(Sender: TObject);
begin
 SFV_Create_Panel.Visible := False;
 Check_Single_Panel.Visible := False;
 Check_All_Panel.Visible := True;
 Check_All_Panel.BringToFront;
end;

procedure Tmainform.ChA_SFV_SelectClick(Sender: TObject);
begin
 if OpenDialog2.Execute then ChA_SFV_FN_Edit.Text := OpenDialog2.FileName;
end;

procedure UnixCheck (FileName : String);
var I, O : File; Ib   : Array [1..32000] of Char;
    Ob   : Array [1..64000] of Char; Ii, Oi, Nr, tmp, doconv   : Integer;
begin

// so what we do here is to check whether the input SFV
// file has only lf instead of cr/lf as end-of-line
// char. because if it has, we can't process it using
// readln, so we first have to convert the lf's into cr/lf's

 AssignFile (I, FileName); Reset (I, 1); doconv := 0; BlockRead(I, Ib, 32000, Nr);
 for tmp := 1 to Nr do begin if Ib[tmp] = Chr(10) then
  if Ib[tmp - 1] <> Chr(13) then doconv := 1; end; CloseFile (I);
 if doconv = 1 then begin
 //MessageBox(0,'Converting SFV file!', 'Unix format', MB_OK);
  AssignFile (I, FileName); Reset (I, 1); AssignFile (O, FileName+'.tmp'); Rewrite (O, 1);
  repeat Oi := 1; BlockRead(I, Ib, 32000, Nr); if Nr <> 0 then
    begin for Ii := 1 to Nr do begin if Ib[Ii] = Chr(10) then
          if Ib[Ii-1] <> Chr(13) then begin Ob[Oi] := Chr(13); inc(Oi); end;
         Ob[Oi] := Ib[Ii]; inc(Oi); end; BlockWrite(O, Ob, Oi); end;
  until (Nr = 0); CloseFile(I); CloseFile(O);
  DeleteFile(FileName); RenameFile(FileName+'.tmp', FileName);

end;

end;


procedure Tmainform.ChA_CheckNowClick(Sender: TObject);
var F: TextFile;
    T, FN, FN2: String[255];
    tot, bad, msg, cnt: Integer;
    C: LongWord;
    nextline: String;
    TF: File;
begin

 if ChA_CheckNow.Caption = 'Abort' then
 begin
   abort_current_operation := 1;
   ChA_CheckNow.Caption := 'Perform SFV Check Now!';
   Application.ProcessMessages;
   Exit;
 end;

 if ChA_SFV_FN_Edit.Text = '' then
 begin
  MessageBox(0, 'You need to select a SFV file first!', 'Small hint :)', MB_OK);
  Exit;
 end; // No SFV File selected at all

 if not FileExists(ChA_SFV_FN_Edit.Text) then
 begin
  MessageBox(0, 'The SFV file you specified was not found.', 'Sorry', MB_OK);
  Exit;
 end; // Invalid SFV File specified

  tot := 0; msg := 0; bad := 0;

  abort_current_operation := 0;
  ChA_CheckNow.Caption := 'Abort';

  SetCurrentDir (ExtractFilePath(ChA_SFV_FN_Edit.Text));

  Status2.Clear;
  Status2b.Clear;

  Status2.Lines.Add ('Processing SFV file ' + ExtractFileName(ChA_SFV_FN_Edit.Text) + '...');
  Status2.Lines.Add ('');
  Application.ProcessMessages();
  UnixCheck (ChA_SFV_FN_Edit.Text); // convert line termination if necessary
  AssignFile (F, ChA_SFV_FN_Edit.Text); Reset (F);

  while not eof (F) do
  begin

  readln (F, T);
  if (Pos(';', T) = 0) then   // not a comment ?
  if (Length(T) > 8) then     // valid line contains CRC-String and is > 8 chars in length
  begin

      FN := '';
      for cnt:=1 to Length(T) do
      begin
       if T[cnt] = ' ' then break;
       FN := FN + T[cnt];           // Extract filename from current line in SFV file
      end; FN2 := FN;

      if (abort_current_operation = 0) then
      begin
         Status2.Lines.Add ('Processing ' + FN + '...');
         Application.ProcessMessages();
         inc (tot);
         if FileExists(FN) then
         begin
           C := GetFileCRC (FN, 2);
           if Length(FN) > 12 then FN[0] := chr(12);
           nextline := FN + ' ... local CRC32 = '+Format('0x%08.08X',[C])+' ';

           if (Pos(Format('%08.08X',[C]), UpperCase(T)) = 0) then
           begin
             nextline := nextline + '... failed -> file is BAD';
             inc (bad);
             AssignFile(TF, FN2); Reset(TF,1);
             Status2b.Lines.Add ('BAD    : ' + FN + ' (filesize ' + inttostr(filesize(TF)) +')');
             CloseFile(TF);
           end else
           begin
             nextline := nextline + '... match  -> file is GOOD';
           end
         end else
           begin
             nextline := FN + ' ... local CRC32 = 00000000 ';
             nextline := nextline + '... not found ->   MiSSiNG';
             inc (msg);
             Status2b.Lines.Add ('MISSING: ' + FN2);
           end; // msg
         end;
         Status2.Lines.Delete(Status2.Lines.Count - 1);
         Status2.Lines.Add (nextline);
         Application.ProcessMessages;
      end;
  end;
  CloseFile (F);

  Status2.Lines.Add (' ');
  Status2.Lines.Add ('[ '+IntToStr(tot)+' files processed ] - [ ' +
     IntToStr(tot-bad-msg)+' good ] - [ '+IntToStr(bad)+ ' bad ] - [ ' +
     IntToStr(msg) + ' missing ]');

 Status2.Lines.Add ('');
 Status2.Lines.Add ('Status: Ready...');
 ChA_CheckNow.Caption := 'Perform SFV Check Now!';
 Status2b.Lines.Add ('---');

end;

procedure Tmainform.Image1MouseMove(Sender: TObject; Shift: TShiftState; X,
  Y: Integer);
begin
 LowLightCommandButtons;
end;

procedure Tmainform.Status2bMouseMove(Sender: TObject; Shift: TShiftState;
  X, Y: Integer);
begin
 LowLightCommandButtons;
end;

procedure Tmainform.Status2MouseMove(Sender: TObject; Shift: TShiftState;
  X, Y: Integer);
begin
 LowLightCommandButtons;
end;

procedure Tmainform.Check_All_PanelMouseMove(Sender: TObject;
  Shift: TShiftState; X, Y: Integer);
begin
 LowLightCommandButtons;
end;

procedure Tmainform.cmdLabel3Click(Sender: TObject);
begin
 Check_All_Panel.Visible := False;
 Check_Single_Panel.Visible := True;
 Check_Single_Panel.BringToFront;
end;

procedure Tmainform.SingleSFVFileSelectClick(Sender: TObject);
begin
 if OpenDialog3.Execute then
 begin
  SingleSFVName.Text := OpenDialog3.FileName;
  DirectoryListBox2.Directory := ExtractFilePath(OpenDialog3.FileName);
 end;
end;

procedure Tmainform.FileListBox2DblClick(Sender: TObject);
var answer: LongInt;
begin
 // Add file to ListBox2
 if Pos('.0', FileListBox2.FileName) = 0 then    // RAR-Archive .001 - .0?? !?
  if Pos('.R', UpperCase(FileListBox2.FileName)) = 0 then // RAR-Archive .R?? !?
   if Pos('.M', UpperCase(FileListBox2.FileName)) = 0 then // MP3 File !?
    begin
     answer := Application.MessageBox ('The file is probably not listed in the SFV at all. Add it anyway?', PChar(ExtractFileName(FileListBox2.FileName)), MB_YESNO+MB_DEFBUTTON2+MB_ICONQUESTION);
     if answer <> 6 then Exit;
    end;
 ListBox2.Items.Add(FileListBox2.FileName);
end;

procedure Tmainform.SingleAddAllClick(Sender: TObject);
var cnt: Integer;
begin
 // Add all files to listbox 2
 for cnt := 0 to (FileListBox2.Items.Count - 1) do
  begin
   FileListBox2.ItemIndex := cnt;
   FileListBox2DblClick(self);
  end;
end;

procedure Tmainform.ListBox2DblClick(Sender: TObject);
begin
 // remove single file from target list
 ListBox2.Items.Delete(ListBox2.ItemIndex);
end;

procedure Tmainform.SingleClearListClick(Sender: TObject);
begin
 ListBox2.Items.Clear;
end;

procedure Tmainform.SingleCheckNowClick(Sender: TObject);
var cnt, found, bad: Integer;
    tested: Integer;
    SFV: TextFile;
    MyCRC: LongWord;
    tmp1: string[250];
    nextline: String;
begin
 // check selected files here
 if not FileExists (SingleSFVName.Text) then
 begin
  MessageBox(0, 'Please specify a valid SFV file first!', 'Small hint! :)', MB_OK);
  Exit;
 end;

 if ListBox2.Items.Count = 0 then
 begin
  MessageBox(0, 'Please select at least one file to process!', 'Small hint! :)', MB_OK);
  Exit;
 end;

 tested := 0; bad := 0;
 Status3.Clear;

 UnixCheck (SingleSFVName.Text);

 for cnt := 0 to (ListBox2.Items.Count - 1) do
 begin

  Status3.Lines.Add ('( Processing file ' + ExtractFileName(ListBox2.Items[cnt])+' )');
  Application.ProcessMessages();

  MyCRC := GetFileCRC (listbox2.items[cnt], 3);

  found := 0;
  AssignFile (SFV, SingleSFVName.Text);
  Reset (SFV);
  while not ( eof(sfv) or (found = 1)) do
  begin
   readln(sfv, tmp1);
   tmp1 := UpperCase (tmp1);
   if (Pos(';', tmp1) = 0) then
   begin
    if Pos(UpperCase(ExtractFileName(ListBox2.Items[cnt])),tmp1) <> 0 then
    begin
     found := 1;
     if Pos(Format('%X',[mycrc]),tmp1) <> 0 then
     begin
      nextline := 'GOOD      : ' + ExtractFileName(ListBox2.Items[cnt]);
     end else
     begin
      nextline := 'BAD       : ' + ExtractFileName(ListBox2.Items[cnt]);
      inc(bad);
     end;
    end;
   end;
  end;
  CloseFile(SFV);

  if (found = 0) then
  begin
   nextline := 'NOT LISTED: ' + ExtractFileName(ListBox2.Items[cnt]);
   inc(bad);
  end;

  Status3.Lines.Delete(Status3.Lines.Count - 1);
  Status3.Lines.Add(nextline);
  Application.ProcessMessages();

  inc(tested);
 end;

 Status3.Lines.Add('');
 Status3.Lines.Add('[ '+ inttostr(tested)+' files tested ] - [ '+
     inttostr(tested - bad) + ' good ] - [ ' + inttostr(bad) +
     ' bad/not listed ]');

end;

procedure Tmainform.Image2MouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
// LowLightCommandButtons;
end;

procedure Tmainform.Status3MouseMove(Sender: TObject; Shift: TShiftState;
  X, Y: Integer);
begin
 LowLightCommandButtons;
end;

procedure Tmainform.Image2MouseMove(Sender: TObject; Shift: TShiftState; X,
  Y: Integer);
begin
 LowLightCommandButtons;
end;

procedure Tmainform.ListBox2MouseMove(Sender: TObject; Shift: TShiftState;
  X, Y: Integer);
begin
 LowLightCommandButtons;
end;

procedure Tmainform.DirectoryListBox2MouseMove(Sender: TObject;
  Shift: TShiftState; X, Y: Integer);
begin
 LowLightCommandButtons;
end;

procedure Tmainform.FileListBox2MouseMove(Sender: TObject;
  Shift: TShiftState; X, Y: Integer);
begin
 LowLightCommandButtons;
end;

end.
