
# uberswitch
A header-only, unobtrusive, almighty alternative to the C++ switch statement that looks just like the original.

## Sample usage (incomplete)

```cpp
#include <uberswitch.hpp>

// The holy grail of the switches: the string switch!
int string2num(std::string s) {
    uberswitch(s) {
        case("one"):
            return 1;

        case("two"):
            return 2;

        case("three"):
            return 3;

        // fallthrough works too
        case("four"):
        case("f0ur"):
            return 4

        default: return -1;
    }
}
    
// More unexpected types work too
std::string pair2string(std::pair<int, int> p) {
    uberswitch(p) {
        case(std::make_pair(1, 2)):
            return "12";

        // List initialization works too
        case({3, 4}):
            return "34";

        default:
            return "Unknown pair";
    }
}

// You can actually switch over multiple items without packing them explicitly
std::string pair2string(int a, int b) {
    uberswitch(a, b) {
        case(1, 2):
            return "12";

        case(3, 4):
            return "34";

        // And if you don't care about one of the items, you can use a wildcard
        case(uberswitch::any, 5):
            return "any5";
            
        // Fallthrough works as well.
        case (6, 7):
        case (8, 9):
            return "67 or 89";
            
        default:
            return "Unknown (a,b)";
    }
}


```

