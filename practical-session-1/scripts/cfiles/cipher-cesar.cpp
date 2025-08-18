#include <bits/stdc++.h>
using namespace std;

const int ALFA = 26;

char move(char x, int rot){
	char ans = x;
	if(isalpha(x)){
		char base = 'a';
		if(isupper(x)) base = 'A';
		ans = ((x - base + rot) % ALFA) + base;
	}
	return ans;
}

string cipher_cesar(int rotation, string text, bool decrypt){
	string ans;
	if(decrypt) rotation = ALFA - rotation;
	int n = text.size();
	for(int i = 0; i < n; ++i)
		ans.push_back(move(text[i], rotation));
	return ans;
}

int main(int argc, char const *argv[]){
	if (argc < 4){
		printf("Using <e/d> <rotation> <text>\n");
		return 1;
	}

	string change = argv[1];
	int rotation = stoi(argv[2]) % ALFA;
	string text = argv[3];
	
	string ans = cipher_cesar(rotation, text, (change == "d"));
	cout << ans << endl;
	return 0;
}