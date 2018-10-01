object dlgCOMM: TdlgCOMM
  Left = 350
  Top = 173
  Width = 280
  Height = 235
  ActiveControl = cbPort
  BorderIcons = [biSystemMenu]
  Caption = 'Initialisation (COMM)'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clBlack
  Font.Height = -13
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = True
  Position = poDesktopCenter
  PixelsPerInch = 96
  TextHeight = 16
  object lbPort: TLabel
    Left = 25
    Top = 19
    Width = 24
    Height = 16
    Caption = 'Port'
  end
  object lpBaudrate: TLabel
    Left = 25
    Top = 44
    Width = 55
    Height = 16
    Caption = 'Baudrate'
  end
  object lbParity: TLabel
    Left = 25
    Top = 69
    Width = 34
    Height = 16
    Caption = 'Parity'
  end
  object lbStopbits: TLabel
    Left = 25
    Top = 94
    Width = 52
    Height = 16
    Caption = 'Stop bits'
  end
  object lbBits: TLabel
    Left = 25
    Top = 119
    Width = 22
    Height = 16
    Caption = 'Bits'
  end
  object lbDTR: TLabel
    Left = 25
    Top = 144
    Width = 29
    Height = 16
    Caption = 'DTR'
  end
  object lbRTS: TLabel
    Left = 25
    Top = 169
    Width = 28
    Height = 16
    Caption = 'RTS'
  end
  object cbPort: TComboBox
    Left = 83
    Top = 14
    Width = 75
    Height = 24
    Style = csDropDownList
    ItemHeight = 16
    TabOrder = 0
    Items.Strings = (
      ''
      'COM1'
      'COM2'
      'COM3'
      'COM4'
      'COM5'
      'COM6'
      'COM7'
      'COM8')
  end
  object cbBaudrate: TComboBox
    Left = 83
    Top = 39
    Width = 75
    Height = 24
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlack
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ItemHeight = 16
    ParentFont = False
    TabOrder = 1
    Items.Strings = (
      '110'
      '300'
      '600'
      '1200'
      '2400'
      '2809'
      '4800'
      '9600'
      '14400'
      '19200'
      '38400'
      '57600'
      '115200'
      '128000'
      '256000'
      '512000')
  end
  object btnOk: TButton
    Left = 190
    Top = 160
    Width = 57
    Height = 25
    Caption = 'Ok'
    Default = True
    ModalResult = 1
    TabOrder = 7
  end
  object cbParity: TComboBox
    Left = 83
    Top = 64
    Width = 75
    Height = 24
    Style = csDropDownList
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlack
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ItemHeight = 16
    ParentFont = False
    TabOrder = 2
    Items.Strings = (
      'None'
      'Odd'
      'Even'
      'Mark'
      'Space')
  end
  object cbStopBits: TComboBox
    Left = 83
    Top = 89
    Width = 75
    Height = 24
    Style = csDropDownList
    ItemHeight = 16
    TabOrder = 3
    Items.Strings = (
      '1'
      '1.5'
      '2')
  end
  object cbBits: TComboBox
    Left = 83
    Top = 114
    Width = 75
    Height = 24
    Style = csDropDownList
    ItemHeight = 16
    TabOrder = 4
    Items.Strings = (
      '4'
      '5'
      '6'
      '7'
      '8')
  end
  object cbDTR: TComboBox
    Left = 83
    Top = 139
    Width = 75
    Height = 24
    Style = csDropDownList
    ItemHeight = 16
    TabOrder = 5
    Items.Strings = (
      'false'
      'true')
  end
  object cbRTS: TComboBox
    Left = 83
    Top = 164
    Width = 75
    Height = 24
    Style = csDropDownList
    ItemHeight = 16
    TabOrder = 6
    Items.Strings = (
      'false'
      'true')
  end
  object btnCancel: TButton
    Left = 190
    Top = 120
    Width = 57
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 8
  end
end
