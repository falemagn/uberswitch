
# uberswitch
A header-only, unobtrusive, almighty alternative to the C++ switch statement that looks just like the original.

## Sample usage (incomplete)

```cpp

// Enable nesting
#define UBERSWITCH_ALLOW_NESTING 1

// Include the tool 
#include <uberswitch/uberswitch.hpp>

// The holy grail of the switches: the string switch!
int string2num(std::string s) {
    uswitch (s) {
        ucase ("one"):
            return 1;

        ucase ("two"):
            return 2;

        ucase ("three"):
            return 3;

        // fallthrough works too
        ucase ("four"):
        ucase ("f0ur"):
            return 4;

        default: return -1;
    }
}
    
// More unexpected types work too
std::string pair2string(std::pair<int, int> p) {
    uswitch (p) {
        ucase (std::make_pair(1, 2)):
            return "12";

        // List initialization works too
        ucase ({3, 4}):
            return "34";

        default:
            return "Unknown pair";
    }
}

// You can actually switch over multiple items without packing them explicitly
// and the whole construct is constexpr!
constexpr const char* pair2string(int a, int b) {
    uswitch (a, b) {
        ucase (1, 2):
            return "12";

        ucase (3, 4):
            return "34";

        // And if you don't care about one of the items, you can use a wildcard
        ucase (uberswitch::any, 5):
            return "any5";
            
        // Fallthrough works as well.
        ucase (6, 7):
        ucase (8, 9):
            return "67 or 89";
            
        // And you can of course break out of the switch at any time.
        ucase (0, 0):
            break;
            
        default:
            return "Unknown (a,b)";
    }
    
    return "You found the code to break out!";
}

// Uberswitches can be nested if you define UBERSWITCH_ALLOW_NESTING to 1.
// In that case, fameta::counter will be used to get access to compile time scoped counters, which are needed for the nesting functionality.
constexpr const char* pair2string(int a, int b, int c) {
    uswitch (a, b) {
        ucase (1, 2):
            return "12";

        ucase (3, 4):
            return "34";

        default:
            // Starting a new uberswitch here!
            uberswitch (a, c) {
                ucase (3, 5):
                    return "35";
                    
                // And you can of course also break out of a nested switch.
                ucase (0, 0):
                    break;

                default: 
                    return "Unknown (a,b)";
            }
            
            break;        
    }
    
    return "You found the code to break out!";
}

// Within the standard uberswitch the continue keyword acts just like the break keyword, which makes it unusable for its intended purpose.
// However, for those cases in which such a functionality is required, uberswitch_c and case_c can be used. 
// The trailing 'c' stands for 'context', which is an identifier or a number used as the first parameter of both and needs to be kept in synch between them. 
// Uberswitch_c and case_c cannot be used in constexpr functions and require c++17. This is a necessary cost to be able to use the continue keyword as it was intended.
std::string pairs_in_map(const std::map<int, std::string> &map) {
    std::string ret;
    
    for (const auto &p: map) {
        uswitch_c (M, p.first, p.second) {
            ucase_c (M, 1, "2"):
                ret.append("12");
                break;

            ucase_c (M, 3, "4"):
                ret.append("34");
                break;

            default:
                ret.append("[").append(std::to_string(p.first)).append(p.second).append("]");
                continue;
        }
        
        ret.append("-");
    }
        
    return ret;
}

// The above function, given the following map as input:
//
//     std::map<int, std::string> m {
//         { 2, "4"},
//         { 1, "2"},
//         { 5, "6"},
//         { 3, "4"},
//         { 7, "8"}
//    };
//
// Produces the following output:
//
//     12-[24]34-[56][78]    

```

