
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
        case(std::make_pair(1, 2):
	        return "12";

		case(std::make_pair(3, 4):
			return "34";

		default:
			return "Unknown pair";
    }
}
```

