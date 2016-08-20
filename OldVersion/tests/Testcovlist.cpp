#include "covstdlib.h"
#include <iostream>
#include <string>
int main()
{
    using namespace std;
    cov::list < string > usrinput;
    bool input = true;
    while (input) {
        cout << "Input:";
        string in;
        cin >> in;
        SWITCH(in) {
            DEFAULT {
                usrinput.emplace_back(in);
            }
            BREAK;
            CASE("#PFT") {
                cin >> in;
                usrinput.push_front(in);
            }
            BREAK;
            CASE("#PPB") {
                usrinput.pop_back();
            }
            BREAK;
            CASE("#PPF") {
                usrinput.pop_front();
            }
            BREAK;
            CASE("#DEL") {
                cin >> in;
                usrinput.remove_if([&](const std::string& str) {
                    return str==in;
                });
            }
            BREAK;
            CASE("#CLS") {
                usrinput.clear();
            }
            BREAK;
            CASE("#INS") {
                cin >> in;
                for (auto it = usrinput.begin(); it != usrinput.end(); ++it) {
                    if (*it == in) {
                        cin >> in;
                        usrinput.insert(it, in);
                        break;
                    }
                }
            }
            BREAK;
            CASE("#EXIT") {
                input = false;
            }
            BREAK;
        }
        ENDSWITCH;
        cout << "List Size=" << usrinput.size() << endl;
        cout << "List Elements:" << endl;
        for (auto & it:usrinput) {
            cout << "Element:" << it << endl;
        }
    }
    return 0;
}
