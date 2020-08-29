
# uberswitch
A header-only, unobtrusive, almighty alternative to the C++ switch statement that looks just like the original.

## Sample usage (incomplete)

```cpp
#include <uberswitch/uberswitch.hpp>

// The holy grail of the switches: the string switch!
int string2num(std::string s) {
    uberswitch (s) {
        case ("one"):
            return 1;

        case ("two"):
            return 2;

        case ("three"):
            return 3;

        // fallthrough works too
        case ("four"):
        case ("f0ur"):
            return 4

        default: return -1;
    }
}
    
// More unexpected types work too
std::string pair2string(std::pair<int, int> p) {
    uberswitch (p) {
        case (std::make_pair(1, 2)):
            return "12";

        // List initialization works too
        case ({3, 4}):
            return "34";

        default:
            return "Unknown pair";
    }
}

// You can actually switch over multiple items without packing them explicitly
// and the whole construct is constexpr!
constexpr const char* pair2string(int a, int b) {
    uberswitch (a, b) {
        case (1, 2):
            return "12";

        case (3, 4):
            return "34";

        // And if you don't care about one of the items, you can use a wildcard
        case (uberswitch::any, 5):
            return "any5";
            
        // Fallthrough works as well.
        case (6, 7):
        case (8, 9):
            return "67 or 89";
            
        // And you can of course break out of the switch at any time.
        case (0, 0):
            break;
            
        default:
            return "Unknown (a,b)";
    }
    
    return "You found the code to break out!";
}

// Uberswitches can be nested if you define UBERSWITCH_ALLOW_NESTING to 1.
// In that case, fameta::counter will be used to get access to compile time scoped counters, which are needed for the nesting functionality.
constexpr const char* pair2string(int a, int b, int c) {
    uberswitch (a, b) {
        case (1, 2):
            return "12";

        case (3, 4):
            return "34";

        default:
            // Starting a new uberswitch here!
            uberswitch (a, c) {
                case (3, 5):
                    return "35";
                    
                // And you can of course also break out of a nested switch.
                case (0, 0):
                    break;

                default: 
                    return "Unknown (a,b)";
            }
            
            break;        
    }
    
    return "You found the code to break out!";
}

```

