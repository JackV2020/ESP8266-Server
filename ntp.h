/* ----------------------------------------------------------------------------

ntp.h

  The ntp template file which is created by ntpInit() when it does not exist.

---------------------------------------------------------------------------- */ 

const char ntp_defaults[] PROGMEM = R"rawliteral(#
# ntp configuration with default settings For Amsterdam
#
# Changes to this file need a reboot to be effective.
#
Summer # tzName name of the time zone for summer time, I use Summer, not CEST
0      # week Last, First, Second, Third, Fourth (0 - 4)
0      # wday Sun, Mon, Tue, Wed, Thu, Fri, Sat (0 - 7)
3      # month Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec(0 -11)
2      # hour the local hour when rule chages
120    # tzOffset sum of timezone offset to GMT (=UTC=WET) + summertime offset
#       for Amsterdam : +1 GMT + 1h summertime offset = 120
#
#
Winter # tzName name of the time zone for winter time, I use Winter, not CET
0      # week Last, First, Second, Third, Fourth (0 - 4)
0      # wday Sun, Mon, Tue, Wed, Thu, Fri, Sat (0 - 7)
9      # month Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec(0 -11)
3      # hour the local hour when rule chages
60     # tzOffset timezone offset to GMT (=UTC=WET)
#        for Amsterdam : +1 GMT = 60
#
# End of this file
#
)rawliteral";

// ----------------------------------------------------------------------------