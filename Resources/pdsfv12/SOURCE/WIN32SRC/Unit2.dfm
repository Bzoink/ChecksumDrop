object Form2: TForm2
  Left = 240
  Top = 150
  BorderStyle = bsDialog
  Caption = '[SFV Viewer]'
  ClientHeight = 431
  ClientWidth = 681
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Memo1: TMemo
    Left = 0
    Top = 0
    Width = 681
    Height = 385
    BorderStyle = bsNone
    Color = clNavy
    Font.Charset = ANSI_CHARSET
    Font.Color = clWhite
    Font.Height = -11
    Font.Name = 'Courier New'
    Font.Style = []
    ParentFont = False
    ScrollBars = ssVertical
    TabOrder = 0
  end
  object Button1: TButton
    Left = 0
    Top = 392
    Width = 361
    Height = 33
    Caption = 'Perform &SFV Check'
    TabOrder = 1
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 360
    Top = 392
    Width = 321
    Height = 33
    Caption = '&Exit'
    TabOrder = 2
    OnClick = Button2Click
  end
end
