#include <iostream> 
#include<chrono> 
#include <windows.h>
#include <vector>
#include <algorithm>

#define X first
#define Y second
using namespace std;
int nScreenWidth = 120;
int nScreenHeight = 40;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.14159 / 4;
float fDepth = 16.0f;
float fSpeed = 5.0f;

int main() {

	// 화면 버퍼
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;
	//Map
	wstring map;  // Wolfenstein3D
	map += L"#########.......";
	map += L"#...............";
	map += L"#.......########";
	map += L"#..............#";
	map += L"#......##......#";
	map += L"#......##......#";
	map += L"#..............#";
	map += L"###............#";
	map += L"##.............#";
	map += L"#......#####.###";
	map += L"#......#.......#";
	map += L"#......#.......#";
	map += L"#..............#";
	map += L"#......#########";
	map += L"#..............#";
	map += L"################";

	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	// game Loop
	while (1) {


		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();

		//Controls
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			fPlayerA -= (fSpeed * 0.75f) * fElapsedTime;
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			fPlayerA += (fSpeed * 0.75f) * fElapsedTime;
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
			fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;
			fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
			{
				fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;
				fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;
			}
		}
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
			fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;
			fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;

			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
			{
				fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;
				fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;
			}
		}

		// potter Algorithm
		for (int x = 0; x < nScreenWidth; x++) {

			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;
			float fStepSize = 0.1f;
			float fDistanceToWall = 0;
			float fEyeX = sinf(fRayAngle);
			float fEyeY = cosf(fRayAngle);

			bool bHitWall = false;
			bool bBoundary = false;

			while (!bHitWall && fDistanceToWall < fDepth) {

				fDistanceToWall += fStepSize;

				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

				if (nTestX < 0 || nTestY < 0 || nTestX >= nMapWidth || nTestY >= nMapHeight) {
					bHitWall = true;
					fDistanceToWall = fDepth;
				}
				else
				{
					if (map.c_str()[nTestX * nMapWidth + nTestY] == '#') {
						bHitWall = true;

						vector<pair<float, float>> p; // distance, dot (to find corner)
						for (int tx = 0; tx < 2; tx++) {
							for (int ty = 0; ty < 2; ty++) {
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + ty - fPlayerX;
								float d = sqrt(vx * vx + vy * vy);
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
								p.push_back(make_pair(d, dot));
							}
							sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.X < right.X;});

							float fBound = 0.01;
							if (acos(p.at(0).Y) < fBound) bBoundary = true;
							if (acos(p.at(1).Y) < fBound) bBoundary = true;
							if (p.size() > 2 && acos(p.at(2).second) < fBound) bBoundary = true;

						}

					}
				}
			}

			// Calculate distance to Celling and Floor
			int nCelling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCelling;

			short nShade = ' ';

			if (fDistanceToWall <= fDepth / 4.0f)			nShade = 0x2588; // Unicode(block elemental) Very close
			else if (fDistanceToWall < fDepth / 3.0f)		nShade = 0x2593;
			else if (fDistanceToWall < fDepth / 2.0f)		nShade = 0x2592;
			else if (fDistanceToWall < fDepth)				nShade = 0x2591;
			else											nShade = ' '; // too far

			if (bBoundary)		nShade = ' '; // Black

			for (int y = 0; y < nScreenHeight; y++) {
				if (y <= nCelling)
					screen[y * nScreenWidth + x] = ' ';
				else if (y > nCelling && y <= nFloor)
					screen[y * nScreenWidth + x] = nShade;
				else {
					float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.25)		nShade = '#';
					else if (b < 0.5)	nShade = 'x';
					else if (b < 0.75)	nShade = '.';
					else if (b < 0.9)	nShade = '-';
					else				nShade = ' ';
					screen[y * nScreenWidth + x] = nShade;
				}
			}
		}


		// Display stats
		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

		// Display map
		for (int nx = 0; nx < nMapWidth; nx++)
			for (int ny = 0; ny < nMapWidth; ny++)
			{
				screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
			}
		screen[((int)fPlayerX + 1) * nScreenWidth + (int)fPlayerY] = 'P';

		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}


	return 0;
}

// https://www.youtube.com/watch?v=xW8skO7MFYw&list=PL36enNxU148RzQ8zwL8FHgg7ef_-5p3cQ