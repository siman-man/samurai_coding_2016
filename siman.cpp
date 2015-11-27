#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <limits.h>
#include <string>
#include <string.h>
#include <sstream>
#include <set>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <stack>
#include <queue>

using namespace std;

typedef long long ll;

const int UNKNOWN = -1;

struct PLAYER{
	int id;				// サムライ番号
	int y;				// 現在のy座標
	int x;				// 現在のx座標
	int homeY;		// 居館のy座標
	int homeX; 		// 居館のx座標

	PLAYER(int id = UNKNOWN){
		this->id = id;
		this->y = UNKNOWN;
		this->x = UNKNOWN;
		this->homeY = UNKNOWN;
		this->homeX = UNKNOWN;
	}

	void setHomePosition(int y, int x){
		this->y = y;
		this->x = x;
		this->homeY = y;
		this->homeX = x;
	}
};

// 参加するプレイヤーの最大人数
const int MAX_PLAYER_NUM = 6;

// プレイヤーのリスト
PLAYER g_playerList[MAX_PLAYER_NUM];

// 自分のプレイヤー番号
int g_playerId;
// グループID
int g_groupId;
// 最大ターン数
int g_max_turn;
// フィールドの横幅
int g_width;
// フィールドの縦幅
int g_height;

class SamurAI{
	public:
		/**
		 * 初期情報の読み込みを行う
		 */
		void init(){
			scanf("%d %d %d %d %d", &g_max_turn, &g_playerId, &g_groupId, &g_width, &g_height);

			// 居館の位置を取得（ついでにユーザの初期位置を設定）
			for(int id = 0; id < 6; id++){
				int homeY;
				int homeX;

				PLAYER player(id);
				scanf("%d %d", &homeY, &homeX);

				// プレイヤーの初期位置を初期化
				player.setHomePosition(homeY, homeX);
			}

			// 各プレイヤーの戦績を取得
			for(int id = 0; id < 6; id++){
				int rank;
				int score;

				PLAYER *player = getPlayer(id);
				scanf("%d %d", &rank, &score);
			}

			// 準備完了の通知をサーバ側に足して行う
			cout << 0 << endl;
		}

		void run(){
		}

		PLAYER *getPlayer(int id){
			return &g_playerList[id];
		}
};

int main(){
	SamurAI player;
	player.init();
	player.run();
  return 0;
}
