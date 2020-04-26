#include <bits/stdc++.h>
using namespace std;

int main() {
	cout << "ClickIDE has now updated to 5.x version, let's see what things are improved!\n" << endl;
	string updates[5] = {
		"We have the real-time highlights! That's the biggest improvement of this version.",
		"Current line highlight added. That it is easier to find out the line number.",
		"More stable. Less crashes!",
		"Bug fixed. Better using experience!",
		"Dynamic linking appended. Starts quicker! (So: Dependencies are needed. therefore you need to clone the whole repo."
	};
	for (int i = 0; i < 5; i++) {
		printf("%d. %s\n", i + 1, updates[i].c_str());
	}
	cout << "\nDo you love it? Please download at once!" << endl;
	system("pause");
	return 0;
}