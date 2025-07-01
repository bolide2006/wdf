/*++

Module Name:

    Trace.h

Abstract:

    Header file for the debug tracing related function defintions and macros.

Environment:

    Kernel mode

--*/

//
// Define the tracing flags.
//
// Tracing GUID - ee39552c-f452-47a0-9e7f-8530e89b403f
//

#define WPP_CONTROL_GUIDS                                              \
    WPP_DEFINE_CONTROL_GUID(                                           \
        wdfvpuTraceGuid, (ee39552c,f452,47a0,9e7f,8530e89b403f), \
                                                                            \
        WPP_DEFINE_BIT(MYDRIVER_ALL_INFO)                              \
        WPP_DEFINE_BIT(TRACE_DRIVER)                                   \
        WPP_DEFINE_BIT(TRACE_DEVICE)                                   \
        WPP_DEFINE_BIT(TRACE_QUEUE)                                    \
        WPP_DEFINE_BIT(FlagDriverWideLog)                              \
        WPP_DEFINE_BIT(FlagCallStack)                                  \
        )

#define WPP_FLAG_LEVEL_LOGGER(flag, level)                                  \
    WPP_LEVEL_LOGGER(flag)

#define WPP_FLAG_LEVEL_ENABLED(flag, level)                                 \
    (WPP_LEVEL_ENABLED(flag) &&                                             \
     WPP_CONTROL(WPP_BIT_ ## flag).Level >= level)

#define WPP_LEVEL_FLAGS_LOGGER(lvl,flags) \
           WPP_LEVEL_LOGGER(flags)

#define WPP_LEVEL_FLAGS_ENABLED(lvl, flags) \
           (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= lvl)

//
// WPP orders static parameters before dynamic parameters. To support the Trace function
// defined below which sets FLAGS=MYDRIVER_ALL_INFO, a custom macro must be defined to
// reorder the arguments to what the .tpl configuration file expects.
//
#define WPP_RECORDER_FLAGS_LEVEL_ARGS(flags, lvl) WPP_RECORDER_LEVEL_FLAGS_ARGS(lvl, flags)
#define WPP_RECORDER_FLAGS_LEVEL_FILTER(flags, lvl) WPP_RECORDER_LEVEL_FLAGS_FILTER(lvl, flags)

//
// This comment block is scanned by the trace preprocessor to define our
// Trace function.
//
// begin_wpp config
// FUNC Trace{FLAGS=MYDRIVER_ALL_INFO}(LEVEL, MSG, ...);
// FUNC TraceEvents(LEVEL, FLAGS, MSG, ...);
// end_wpp
//

// This block defines the function entry and exit functions
//
// begin_wpp config
// FUNC TraceEntry();
// FUNC TraceExit();
// USESUFFIX(TraceEntry, "==>%!FUNC!");
// USESUFFIX(TraceExit, "<==%!FUNC!");
// end_wpp
//
#define WPP__ENABLED() WPP_LEVEL_ENABLED(FlagCallStack)
#define WPP__LOGGER() WPP_LEVEL_LOGGER(FlagCallStack)

//
// This block defines a CHK_NT_MSG macro that:
// 1. Checks the NTSTATUS message in the first parameter
// 2. Logs status message to WPP and ETW, if the status indicates a failure
// 3. Jumps to label "End"
//
// begin_wpp config
// FUNC CHK_NT_MSG{CHK=DUMMY,LEVEL=TRACE_LEVEL_ERROR,FLAGS=FlagDriverWideLog}(STATUS, MSG, ...);
// USESUFFIX(CHK_NT_MSG, "[%!STATUS!]", STATUS);
// end_wpp
//
#define WPP_CHK_LEVEL_FLAGS_STATUS_PRE(CHK,LEVEL,FLAGS,STATUS) { \
    if (NT_SUCCESS(STATUS) != TRUE) {

//need add TracePrint
#define WPP_CHK_LEVEL_FLAGS_STATUS_POST(CHK,LEVEL,FLAGS,STATUS) ;\
        goto End; \
    } }
#define WPP_CHK_LEVEL_FLAGS_STATUS_ENABLED(chk,level,flags,status) WPP_LEVEL_ENABLED(flags)
#define WPP_CHK_LEVEL_FLAGS_STATUS_LOGGER(chk,level,flags,status) WPP_LEVEL_LOGGER(flags)

//
// This block defines a LOG_NT_MSG macro that:
// 1. Checks the NTSTATUS message in the first parameter
// 2. And just logs the status message to WPP and ETW, if it indicates a failure
//
// begin_wpp config
// FUNC LOG_NT_MSG{LOG=DUMMY,LEVEL=TRACE_LEVEL_ERROR,FLAGS=FlagDriverWideLog}(STATUS, MSG, ...);
// USESUFFIX(CHK_NT_MSG, "[%!STATUS!]", STATUS);
// end_wpp
//
#define WPP_LOG_LEVEL_FLAGS_STATUS_PRE(CHK,LEVEL,FLAGS,STATUS) { \
    if (NT_SUCCESS(STATUS) != TRUE) {

//need add TracePrint
#define WPP_LOG_LEVEL_FLAGS_STATUS_POST(CHK,LEVEL,FLAGS,STATUS) ;\
    } }

#define WPP_LOG_LEVEL_FLAGS_STATUS_ENABLED(chk,level,flags,status) WPP_LEVEL_ENABLED(flags)
#define WPP_LOG_LEVEL_FLAGS_STATUS_LOGGER(chk,level,flags,status) WPP_LEVEL_LOGGER(flags)

//
// This should optimize WPP macros by disbaling checking for 'WPP_INIT_TRACING'
// (okay, because we enable WPP tracing in Driver entry)
//
#define WPP_CHECK_INIT
