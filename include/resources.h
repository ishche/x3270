/*
 * Copyright (c) 1995-2024 Paul Mattes.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the names of Paul Mattes nor the names of his contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY PAUL MATTES "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL PAUL MATTES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *	resources.h
 *		x3270/c3270/s3270/tcl3270 resource and option names.
 */

/* Resources. */
#define ResAcs			"acs"
#define ResAcceptHostname	"acceptHostname"
#define ResActiveIcon		"activeIcon"
#define ResAdVersion		"adVersion"
#define ResAidWait		"aidWait"
#define ResAlias		"alias"
#define ResAllBold		"allBold"
#define ResAllowResize		"allowResize"
#define ResAltCursor		"altCursor"
#define ResAltScreen		"altScreen"
#define ResAlwaysInsert		"alwaysInsert"
#define ResAplCircledAlpha	"aplCircledAlpha"
#define ResAplMode		"aplMode"
#define ResAsciiBoxDraw		"asciiBoxDraw"
#define ResAssocCommand		"printer.assocCommandLine"
#define ResAutoShortcut		"autoShortcut"
#define ResBaselevelTranslations	"baselevelTranslations"
#define ResBellMode		"bellMode"
#define ResBellVolume		"bellVolume"
#define ResBindUnlock		"bindUnlock"
#define ResBindLimit		"bindLimit"
#define ResBlankFill		"blankFill"
#define ResBoldColor		"boldColor"
#define ResBsdTm		"bsdTm"
#define ResCaDir		"caDir"
#define ResCaFile		"caFile"
#define ResCallback		"callback"
#define ResCbreak		"cbreak"
#define ResCertFile		"certFile"
#define ResCertFileType		"certFileType"
#define ResChainFile		"chainFile"
#define ResCharClass		"charClass"
#define ResCharset		"charset"
#define ResCharsetList		"charsetList"
#define ResClientCert		"clientCert"
#define ResCodePage		"codePage"
#define ResColorBackground	"colorBackground"
#define ResColorScheme		"colorScheme"
#define ResCommandTimeout	"commandTimeout"
#define ResComposeMap		"composeMap"
#define ResConfDir		"confDir"
#define ResConnectFileName	"connectFileName"
#define ResConnectTimeout	"connectTimeout"
#define ResConsole		"console"
#define ResConsoleColorForHostColor "consoleColorForHostColor"
#define ResContentionResolution	"contentionResolution"
#define ResCrosshair		"crosshair"
#define ResCrosshairColor	"crosshairColor"
#define ResCursesColorFor	"cursesColorFor"
#define ResCursesColorForDefault ResCursesColorFor "Default"
#define ResCursesColorForHostColor ResCursesColorFor "HostColor"
#define ResCursesColorForIntensified ResCursesColorFor "Intensified"
#define ResCursesColorForProtected ResCursesColorFor "Protected"
#define ResCursesColorForProtectedIntensified ResCursesColorFor "ProtectedIntensified"
#define ResCursesKeypad		"cursesKeypad"
#define ResCursorBlink		"cursorBlink"
#define ResCursorColor		"cursorColor"
#define ResDbcsCgcsgid		"dbcsCgcsgid"
#define ResDebugTracing		"debugTracing"
#define ResDefaultFgBg		"defaultFgBg"
#define ResDefScreen		"defScreen"
#define ResDevName		"devName"
#define ResDisconnectClear	"disconnectClear"
#define ResDoConfirms		"doConfirms"
#define ResDpi			"dpi"
#define ResEmulatorFont		"emulatorFont"
#define ResEof			"eof"
#define ResErase		"erase"
#define ResFixedSize		"fixedSize"
#define ResFtAllocation		"ftAllocation"
#define ResFtAvblock		"ftAvblock"
#define ResFtBlksize		"ftBlksize"
#define ResFtBufferSize		"ftBufferSize"
#define ResFtCr			"ftCr"
#define ResFtDirection		"ftDirection"
#define ResFtExist		"ftExist"
#define ResFtHost		"ftHost"
#define ResFtHostFile		"ftHostFile"
#define ResFtLocalFile		"ftLocalFile"
#define ResFtLrecl		"ftLrecl"
#define ResFtMode		"ftMode"
#define ResFtOtherOptions	"ftOtherOptions"
#define ResFtPrimarySpace	"ftPrimarySpace"
#define ResFtRecfm		"ftRecfm"
#define ResFtRemap		"ftRemap"
#define ResFtSecondarySpace	"ftSecondarySpace"
#define ResFtWindowsCodePage	"ftWindowsCodePage"
#define ResHighlightBold	"highlightBold"
#define ResHostColorFor		"hostColorFor"
#define ResHostColorForDefault ResHostColorFor "Default"
#define ResHostColorForIntensified ResHostColorFor "Intensified"
#define ResHostColorForProtected ResHostColorFor "Protected"
#define ResHostColorForProtectedIntensified ResHostColorFor "ProtectedIntensified"
#define ResHostname		"hostname"
#define ResHostsFile		"hostsFile"
#define ResHttpd		"httpd"
#define ResIconFont		"iconFont"
#define ResIconLabelFont	"iconLabelFont"
#define ResIcrnl		"icrnl"
#define ResIdleCommand		"idleCommand"
#define ResIdleCommandEnabled	"idleCommandEnabled"
#define ResIdleTimeout		"idleTimeout"
#define ResIndent		"indent"
#define ResInlcr		"inlcr"
#define ResInputColor		"inputColor"
#define ResInputMethod		"inputMethod"
#define ResInsertMode		"insertMode"
#define ResIntr			"intr"
#define ResInvertKeypadShift	"invertKeypadShift"
#define ResJson			"json"
#define ResKeyFile		"keyFile"
#define ResKeyFileType		"keyFileType"
#define ResKeymap		"keymap"
#define ResKeypad		"keypad"
#define ResKeypadBackground	"keypadBackground"
#define ResKeypadOn		"keypadOn"
#define ResKeyPasswd		"keyPasswd"
#define ResKill			"kill"
#define ResLabelIcon		"labelIcon"
#define ResLightPenPrimary	"lightPenPrimary"
#define ResLineWrap		"lineWrap"
#define ResLnext		"lnext"
#define ResLocalCp		"localCp"
#define ResLoginMacro		"loginMacro"
#define ResLockedCursor		"lockedCursor"
#define ResLuCommandLine	"printer.luCommandLine"
#define ResMacros		"macros"
#define ResMarginedPaste	"marginedPaste"
#define ResMaxRecent		"maxRecent"
#define ResMenuBar		"menuBar"
#define ResMetaEscape		"metaEscape"
#define ResMinVersion		"minVersion"
#define ResModel		"model"
#define ResModifiedSel		"modifiedSel"
#define ResModifiedSelColor	"modifiedSelColor"
#define ResMono			"mono"
#define ResMonoCase		"monoCase"
#define ResMouse		"mouse"
#define ResNewEnviron		"newEnviron"
#define ResNoOther		"noOther"
#define ResNoPrompt		"noPrompt"
#define ResNopSeconds		"nopSeconds"
#define ResNoTelnetInputMode	"noTelnetInputMode"
#define ResNormalColor		"normalColor"
#define ResNormalCursor		"normalCursor"
#define ResNumericLock		"numericLock"
#define ResNvtMode		"nvtMode"
#define ResOerrLock		"oerrLock"
#define ResOnce			"once"
#define ResOnlcr		"onlcr"
#define ResOverlayPaste		"overlayPaste"
#define ResOversize		"oversize"
#define ResPort			"port"
#define ResPreeditType		"preeditType"
#define ResPreferIpv4		"preferIpv4"
#define ResPreferIpv6		"preferIpv6"
#define ResPrinterCodepage	"printer.codepage"
#define ResPrinterCommand	"printer.command"
#define ResPrinterLu		"printerLu"
#define ResPrinterName		"printer.name"
#define ResPrinterOptions	"printer.options"
#define ResPrintDialog		"printDialog"
#define ResProxy		"proxy"
#define ResQuit			"quit"
#define ResQrBgColor		"qrBgColor"
#define ResReconnect		"reconnect"
#define ResRectangleSelect	"rectangleSelect"
#define ResRetry		"retry"
#define ResReverseInputMode	"reverseInputMode"
#define ResReverseVideo		"reverseVideo"
#define ResRightToLeftMode	"rightToLeftMode"
#define ResRprnt		"rprnt"
#define ResSaveLines		"saveLines"
#define ResSchemeList		"schemeList"
#define ResScreenTrace		"screenTrace"
#define ResScreenTraceFile	"screenTraceFile"
#define ResScreenTraceTarget	"screenTraceTarget"
#define ResScreenTraceType	"screenTraceType"
#define ResScripted		"scripted"
#define ResScriptedAlways	"scriptedAlways"
#define ResScriptPort		"scriptPort"
#define ResScriptPortOnce	"scriptPortOnce"
#define ResScrollBar		"scrollBar"
#define ResSecure		"secure"
#define ResSelectBackground	"selectBackground"
#define ResSelectUrl		"selectUrl"
#define ResSbcsCgcsgid		"sbcsCgcsgid"
#define ResShowTiming		"showTiming"
#define ResSocket		"socket"
#define ResStartTls		"startTls"
#define ResSuppressActions	"suppressActions"
#define ResSuppressHost		"suppressHost"
#define ResSuppressFontMenu	"suppressFontMenu"
#define ResSuppress		"suppress"
#define ResTermName		"termName"
#define ResTitle		"title"
#define ResTls992		"tls992"
#define ResTlsMaxProtocol	"tlsMaxProtocol"
#define ResTlsMinProtocol	"tlsMinProtocol"
#define ResTrace		"trace"
#define ResTraceDir		"traceDir"
#define ResTraceFile		"traceFile"
#define ResTraceFileSize	"traceFileSize"
#define ResTraceMonitor		"traceMonitor"
#define ResTypeahead		"typeahead"
#define ResUnderscore		"underscore"
#define ResUnderscoreBlankFill	"underscoreBlankFill"
#define ResUnlockDelay		"unlockDelay"
#define ResUnlockDelayMs	"unlockDelayMs"
#define ResUseCursorColor	"useCursorColor"
#define ResUser			"user"
#define ResUtf8			"utf8"
#define ResVerifyHostCert	"verifyHostCert"
#define ResVisibleControl	"visibleControl"
#define ResVisualBell		"visualBell"
#define ResVisualSelect		"visualSelect"
#define ResVisualSelectColor	"visualSelectColor"
#define ResWaitCursor		"waitCursor"
#define ResWerase		"werase"
#define ResWrapperDoc		"wrapperDoc"
#define ResWrongTerminalName	"wrongTerminalName"
#define ResXQuartzHack		"xQuartzHack"

/* Dotted resource names. */
#define DotAcceptHostname	"." ResAcceptHostname
#define DotActiveIcon		"." ResActiveIcon
#define DotAlias		"." ResAlias
#define DotAplMode		"." ResAplMode
#define DotCaDir		"." ResCaDir
#define DotCaFile		"." ResCaFile
#define DotCertFile		"." ResCertFile
#define DotCertFileType		"." ResCertFileType
#define DotChainFile		"." ResChainFile
#define DotCbreak		"." ResCbreak
#define DotCharClass		"." ResCharClass
#define DotClientCert		"." ResClientCert
#define DotCodePage		"." ResCodePage
#define DotColorScheme		"." ResColorScheme
#define DotConnectTimeout	"." ResConnectTimeout
#define DotDevName		"." ResDevName
#define DotEmulatorFont		"." ResEmulatorFont
#define DotHostsFile		"." ResHostsFile
#define DotHttpd		"." ResHttpd
#define DotInputMethod		"." ResInputMethod
#define DotKeyFile		"." ResKeyFile
#define DotKeyFileType		"." ResKeyFileType
#define DotKeymap		"." ResKeymap
#define DotKeypadOn		"." ResKeypadOn
#define DotKeyPasswd		"." ResKeyPasswd
#define DotLoginMacro		"." ResLoginMacro
#define DotTlsMaxProtocol	"." ResTlsMaxProtocol
#define DotTlsMinProtocol	"." ResTlsMinProtocol
#define DotMinVersion		"." ResMinVersion
#define DotModel		"." ResModel
#define DotMono			"." ResMono
#define DotNopSeconds		"." ResNopSeconds
#define DotNvtMode		"." ResNvtMode
#define DotOnce			"." ResOnce
#define DotOversize		"." ResOversize
#define DotPort			"." ResPort
#define DotPreeditType		"." ResPreeditType
#define DotPreferIpv4		"." ResPreferIpv4
#define DotPreferIpv6		"." ResPreferIpv6
#define DotPrinterLu		"." ResPrinterLu
#define DotProxy		"." ResProxy
#define DotReconnect		"." ResReconnect
#define DotSaveLines		"." ResSaveLines
#define DotScripted		"." ResScripted
#define DotScriptPort		"." ResScriptPort
#define DotScriptPortOnce	"." ResScriptPortOnce
#define DotScrollBar		"." ResScrollBar
#define DotSecure		"." ResSecure
#define DotSelfSignedOk		"." ResSelfSignedOk
#define DotSocket		"." ResSocket
#define DotTermName		"." ResTermName
#define DotTitle		"." ResTitle
#define DotTrace		"." ResTrace
#define DotTraceFile		"." ResTraceFile
#define DotTraceFileSize	"." ResTraceFileSize
#define DotUser			"." ResUser
#define DotUtf8			"." ResUtf8
#define DotVerifyHostCert	"." ResVerifyHostCert

/* Resource classes. */
#define ClsAcceptHostname	"AcceptHostname"
#define ClsActiveIcon		"ActiveIcon"
#define ClsAdVersion		"AdVersion"
#define ClsAidWait		"AidWait"
#define ClsAllBold		"AllBold"
#define ClsAllowResize		"AllowResize"
#define ClsAltCursor		"AltCursor"
#define ClsAlwaysInsert		"AlwaysInsert"
#define ClsAplCircledAlpha	"AplCircledAlpha"
#define ClsAplMode		"AplMode"
#define ClsBaselevelTranslations	"BaselevelTranslations"
#define ClsBellVolume		"BellVolume"
#define ClsBindLimit		"BindLimit"
#define ClsBindUnlock		"BindUnlock"
#define ClsBlankFill		"BlankFill"
#define ClsBoldColor		"BoldColor"
#define ClsBsdTm		"BsdTm"
#define ClsCaDir		"CaDir"
#define ClsCaFile		"CaFile"
#define ClsCbreak		"Cbreak"
#define ClsCertFile		"CertFile"
#define ClsCertFileType		"CertFileType"
#define ClsChainFile		"ChainFile"
#define ClsCharClass		"CharClass"
#define ClsCharset		"Charset"
#define ClsClientCert		"ClientCert"
#define ClsCodePage		"CodePage"
#define ClsColorBackground	"ColorBackground"
#define ClsColorScheme		"ColorScheme"
#define ClsComposeMap		"ComposeMap"
#define ClsConfDir		"ConfDir"
#define ClsConnectFileName	"ConnectFileName"
#define ClsConnectTimeout	"ConnectTimeout"
#define ClsConsole		"Console"
#define ClsContentionResolution	"ContentionResolution"
#define ClsCrosshair		"Crosshair"
#define ClsCrosshairColor	"CrosshairColor"
#define ClsCursorBlink		"CursorBlink"
#define ClsCursorColor		"CursorColor"
#define ClsDbcsCgcsgid		"DbcsCgcsgid"
#define ClsDebugTracing		"DebugTracing"
#define ClsDevName		"DevName"
#define ClsDisconnectClear	"DisconnectClear"
#define ClsDoConfirms		"DoConfirms"
#define ClsDpi			"Dpi"
#define ClsEmulatorFont		"EmulatorFont"
#define ClsEof			"Eof"
#define ClsErase		"Erase"
#define ClsFixedSize		"FixedSize"
#define ClsFtAllocation		"FtAllocation"
#define ClsFtAvblock		"FtAvblock"
#define ClsFtBlksize		"FtBlksize"
#define ClsFtBufferSize		"FtBufferSize"
#define ClsFtCr			"FtCr"
#define ClsFtDirection		"FtDirection"
#define ClsFtExist		"FtExist"
#define ClsFtHost		"FtHost"
#define ClsFtHostFile		"FtHostFile"
#define ClsFtLocalFile		"FtLocalFile"
#define ClsFtLrecl		"FtLrecl"
#define ClsFtMode		"FtMode"
#define ClsFtOtherOptions	"FtOtherOptions"
#define ClsFtPrimarySpace	"FtPrimarySpace"
#define ClsFtRecfm		"FtRecfm"
#define ClsFtRemap		"FtRemap"
#define ClsFtSecondarySpace	"FtSecondarySpace"
#define ClsFtWindowsCodePage	"FtWindowsCodePage"
#define ClsHighlightBold	"HighlightBold"
#define ClsHostname		"Hostname"
#define ClsHostsFile		"HostsFile"
#define ClsHttpd		"Httpd"
#define ClsIconFont		"IconFont"
#define ClsIconLabelFont	"IconLabelFont"
#define ClsIcrnl		"Icrnl"
#define ClsIdleCommand		"IdleCommand"
#define ClsIdleCommandEnabled	"IdleCommandEnabled"
#define ClsIdleTimeout		"IdleTimeout"
#define ClsInlcr		"Inlcr"
#define ClsInputColor		"InputColor"
#define ClsInputMethod		"InputMethod"
#define ClsInsertMode		"InsertMode"
#define ClsIntr			"Intr"
#define ClsInvertKeypadShift	"InvertKeypadShift"
#define ClsKeyFile		"KeyFile"
#define ClsKeyFileType		"KeyFileType"
#define ClsKeymap		"Keymap"
#define ClsKeypad		"Keypad"
#define ClsKeypadBackground	"KeypadBackground"
#define ClsKeypadOn		"KeypadOn"
#define ClsKeyPasswd		"KeyPasswd"
#define ClsKill			"Kill"
#define ClsLabelIcon		"LabelIcon"
#define ClsLineWrap		"LineWrap"
#define ClsLnext		"Lnext"
#define ClsLockedCursor		"LockedCursor"
#define ClsLoginMacro		"LoginMacro"
#define ClsMacros		"Macros"
#define ClsMarginedPaste	"MarginedPaste"
#define ClsMaxRecent		"MaxRecent"
#define ClsMenuBar		"MenuBar"
#define ClsMetaEscape		"MetaEscape"
#define ClsMinVersion		"MinVersion"
#define ClsModel		"Model"
#define ClsModifiedSel		"ModifiedSel"
#define ClsModifiedSelColor	"ModifiedSelColor"
#define ClsMono			"Mono"
#define ClsMonoCase		"MonoCase"
#define ClsNewEnviron		"NewEnviron"
#define ClsNoOther		"NoOther"
#define ClsNoTelnetInputMode	"NoTelnetInputMode"
#define ClsNopSeconds		"NopSeconds"
#define ClsNormalColor		"NormalColor"
#define ClsNormalCursor		"NormalCursor"
#define ClsNumericLock		"NumericLock"
#define ClsNvtMode		"NvtMode"
#define ClsOerrLock		"OerrLock"
#define ClsOnce			"Once"
#define ClsOnlcr		"Onlcr"
#define ClsOverlayPaste		"OverlayPaste"
#define ClsOversize		"Oversize"
#define ClsPort			"Port"
#define ClsPreeditType		"PreeditType"
#define ClsPreferIpv4		"PreferIpv4"
#define ClsPreferIpv6		"PreferIpv6"
#define ClsPrinterLu		"PrinterLu"
#define ClsProxy		"Proxy"
#define ClsQuit			"Quit"
#define ClsReconnect		"Reconnect"
#define ClsRectangleSelect	"RectangleSelect"
#define ClsRetry		"Retry"
#define ClsReverseInputMode	"ReverseInputMode"
#define ClsRightToLeftMode	"RightToLeftMode"
#define ClsRprnt		"Rprnt"
#define ClsSaveLines		"SaveLines"
#define ClsSbcsCgcsgid		"SbcsSgcsgid"
#define ClsScreenTrace		"ScreenTrace"
#define ClsScreenTraceFile	"ScreenTraceFile"
#define ClsScreenTraceTarget	"ScreenTraceTarget"
#define ClsScreenTraceType	"ScreenTraceType"
#define ClsScripted		"Scripted"
#define ClsScriptedAlways	"ScriptedAlways"
#define ClsScriptPort		"ScriptPort"
#define ClsScriptPortOnce	"ScriptPortOnce"
#define ClsScrollBar		"ScrollBar"
#define ClsSecure		"Secure"
#define ClsSelectBackground	"SelectBackground"
#define ClsSelectUrl		"SelectUrl"
#define ClsShowTiming		"ShowTiming"
#define ClsSocket		"Socket"
#define ClsStartTls		"StartTls"
#define ClsSuppressActions	"SuppressActions"
#define ClsSuppressHost		"SuppressHost"
#define ClsSuppressFontMenu	"SuppressFontMenu"
#define ClsTermName		"TermName"
#define ClsTls992		"Tls992"
#define ClsTlsMaxProtocol	"TlsMaxProtocol"
#define ClsTlsMinProtocol	"TlsMinProtocol"
#define ClsTrace		"Trace"
#define ClsTraceDir		"TraceDir"
#define ClsTraceFile		"TraceFile"
#define ClsTraceFileSize	"TraceFileSize"
#define ClsTraceMonitor		"TraceMonitor"
#define ClsTypeahead		"Typeahead"
#define ClsUnderscoreBlankFill	"UnderscoreBlankFill"
#define ClsUnlockDelay		"UnlockDelay"
#define ClsUnlockDelayMs	"UnlockDelayMs"
#define ClsUseCursorColor	"UseCursorColor"
#define ClsUser			"User"
#define ClsUtf8			"Utf8"
#define ClsVerifyHostCert	"VerifyHostCert"
#define ClsVisibleControl	"VisibleControl"
#define ClsVisualBell		"VisualBell"
#define ClsVisualSelect		"VisualSelect"
#define ClsVisualSelectColor	"VisualSelectColor"
#define ClsWaitCursor		"WaitCursor"
#define ClsWerase		"Werase"
#define ClsWrongTerminalName	"WrongTerminalName"
#define ClsXQuartzHack		"XQuartzHack"

/* Options. */
#define OptAcceptHostname	"-accepthostname"
#define OptActiveIcon		"-activeicon"
#define OptAlias		"-alias"
#define OptAllBold		"-allbold"
#define OptAltScreen		"-altscreen"
#define OptAplMode		"-apl"
#define OptCaDir		"-cadir"
#define OptCaFile		"-cafile"
#define OptCallback		"-callback"
#define OptCbreak		"-cbreak"
#define OptCertFile		"-certfile"
#define OptCertFileType		"-certfiletype"
#define OptChainFile		"-chainfile"
#define OptCharClass		"-cc"
#define OptCharset		"-charset"
#define OptClientCert		"-clientcert"
#define OptClear		"-clear"
#define OptCodePage		"-codepage"
#define OptColorScheme		"-scheme"
#define OptConnectTimeout	"-connecttimeout"
#define OptDefaultFgBg		"-defaultfgbg"
#define OptDefScreen		"-defscreen"
#define OptDevName		"-devname"
#define OptEmulatorFont		"-efont"
#define OptHostsFile		"-hostsfile"
#define OptHelp1		"--help"
#define OptHelp2		"-?"
#define OptHelp3		"/?"
#define OptHttpd		"-httpd"
#define OptIconName		"-iconname"
#define OptIconX		"-iconx"
#define OptIconY		"-icony"
#define OptIndent		"-indent"
#define OptInputMethod		"-im"
#define OptJson			"-json"
#define OptKeyFile		"-keyfile"
#define OptKeyFileType		"-keyfiletype"
#define OptKeymap		"-keymap"
#define OptKeypadOn		"-keypad"
#define OptKeyPasswd		"-keypasswd"
#define OptLocalCp		"-localcp"
#define OptLocalProcess		"-e"
#define OptLoginMacro		"-loginmacro"
#define OptMinVersion		"-minversion"
#define OptModel		"-model"
#define OptMono			"-mono"
#define OptNoPrompt		"-noprompt"
#define OptNoScrollBar		"+sb"
#define OptNoWrapperDoc		"-nowrapperdoc"
#define OptNvtMode		"-nvt"
#define OptOnce			"-once"
#define OptOversize		"-oversize"
#define OptPort			"-port"
#define OptPreeditType		"-pt"
#define OptPreferIpv4		"-4"
#define OptPreferIpv6		"-6"
#define OptPrinterLu		"-printerlu"
#define OptProxy		"-proxy"
#define OptReconnect		"-reconnect"
#define OptReverseVideo		"-rv"
#define OptSaveLines		"-sl"
#define OptSecure		"-secure"
#define OptScripted		"-script"
#define OptScriptPort		"-scriptport"
#define OptScriptPortOnce	"-scriptportonce"
#define OptScrollBar		"-sb"
#define OptSet			"-set"
#define OptSocket		"-socket"
#define OptSyncPort		"-syncport"
#define OptAutoShortcut		"-S"
#define OptNoAutoShortcut	"+S"
#define OptNoVerifyHostCert	"-noverifycert"
#define OptTermName		"-tn"
#define OptTitle		"-title"
#define OptTlsMaxProtocol	"-tlsmaxprotocol"
#define OptTlsMinProtocol	"-tlsminprotocol"
#define OptTrace		"-trace"
#define OptTraceFile		"-tracefile"
#define OptTraceFileSize	"-tracefilesize"
#define OptUser			"-user"
#define OptUtf8			"-utf8"
#define OptV			"-v"
#define OptVerifyHostCert	"-verifycert"
#define OptVersion		"--version"
#define OptXml			"-xml"
#define OptXrm			"-xrm"

/* Miscellaneous values. */
#define ResTrue			"true"
#define ResFalse		"false"
#define KpLeft			"left"
#define KpRight			"right"
#define KpBottom		"bottom"
#define KpIntegral		"integral"
#define KpInsideRight		"insideRight"
#define Apl			"apl"

/* Resources that are gotten explicitly. */
#define ResComposeMap		"composeMap"
#define ResEmulatorFontList	"emulatorFontList"
#define ResKeyHeight		"keyHeight"
#define ResKeyWidth		"keyWidth"
#define ResLargeKeyWidth	"largeKeyWidth"
#define ResMessage		"message"
#define ResNvt			"nvt"
#define ResPaWidth		"paWidth"
#define ResPfWidth		"pfWidth"
#define ResPrintTextCommand	"printTextCommand"
#define ResPrintTextFont	"printTextFont"
#define ResPrintTextHorizontalMargin "printTextHorizontalMargin"
#define ResPrintTextOrientation	"printTextOrientation"
#define ResPrintTextScreensPerPage "printTextScreensPerPage"
#define ResPrintTextSize	"printTextSize"
#define ResPrintTextVerticalMargin "printTextVerticalMargin"
#define ResPrintWindowCommand	"printWindowCommand"
#define ResUser			"user"

/* Handy macro. */
#define TrueFalse(b)	((b)? ResTrue: ResFalse)
