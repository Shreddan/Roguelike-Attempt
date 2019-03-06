// BWRL.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#
#include "Functions.h"



static std::mt19937 rnd;

#define frand()  std::uniform_real_distribution<>(0.f, 1.f)(rnd)

#define rand(size) std::uniform_int_distribution<>(0, (size)-1)(rnd)

template<typename T, std::size_t size>
constexpr std::size_t count(const T(&)[size])
{
	return size;
}

static std::regex operator ""_r(const char* pattern, std::size_t length)
{
	return std::regex(pattern, length);
}



struct GenericData
{

	const char* name;
	float worth;
	float weight;
} 
static const
MoneyTypes[] =
{ {"platinum", 10, 0.01}, {"gold", 1, 0.01}, {"silver", 0.5, 0.01}, {"copper", 0.2, 0.01} },

EnvTypes[] =
{ {"dark", 0, 0}, {"tall", 0, 0}, {"humid", 0 , 0}, {"beautiful", 0, 0}, {"narrow", 0, 0} };


//Room functions
struct Room
{
	std::size_t Wall = 0, Env = 0;
	unsigned seed = 0;
}static const defaultroom ;



//Maze functions
struct Maze
{
	std::map<long/*x*/, std::map<long/*y*/, Room> > rooms;

	Room& GenerateRoom(long x, long y, const Room& model, unsigned seed)
	{
		rnd.seed(y * 0xc70f6907UL + x * 2166136261UL);
		auto insres = rooms[x].insert({ y, model });
		Room& room = insres.first->second;
		if (insres.second)
		{
			room.seed = (seed + (frand() > 0.95 ? rand(4) : 0)) & 3;

			if (frand() > 0.9) room.Env = rand(count(EnvTypes));
				if (frand() > (seed == model.seed ? 0.95 : 0.1))
					room.Wall = frand() < 0.4 ? 2 : 0;
		}
		return room;
	}

	char Char(long x, long y) const
	{
		auto i = rooms.find(x);  if (i == rooms.end())   return ' ';
		auto j = i->second.find(y); if (j == i->second.end()) return ' ';
		if (j->second.Wall) return '#';
		return '.';
	}

}static maze;

//Character Variables
static long x = 0, y = 0, life = 120;

static bool CanMoveTo(long wherex, long wherey, const Room& model = defaultroom)
{
	if (!maze.GenerateRoom(wherex, wherey, model, 0).Wall) return true;
	return false;
}

static Room& SpawnRooms(long wherex, long wherey, const Room& model = defaultroom)
{
	Room& room = maze.GenerateRoom(wherex, wherey, model, 0);
#define Spawn4rooms(x,y) \
	for (char p: { 1, 3 ,5 ,7}) \
		maze.GenerateRoom(x + p%3-1, y + p/3-1, room, (p+1)/2);
	Spawn4rooms(wherex, wherey);
	for (int o = 1; o < 5 && CanMoveTo(wherex, wherey + o, room); ++o) Spawn4rooms(wherex, wherey + o);
	for (int o = 1; o < 5 && CanMoveTo(wherex, wherey - o, room); ++o) Spawn4rooms(wherex, wherey - o);
	for (int o = 1; o < 6 && CanMoveTo(wherex - o, wherey, room); ++o) Spawn4rooms(wherex - o, wherey);
	for (int o = 1; o < 6 && CanMoveTo(wherex + o, wherey, room); ++o) Spawn4rooms(wherex + o, wherey);
	return room;
}


//Character View function
static void Look()
{
	//Generate map in characters view
	const Room& room = SpawnRooms(x, y);

	//Generate current map view
	std::vector<std::string> mapgraph;
	for (long yo = -4; yo <= 4; ++yo)
	{
		std::string line;
		for (long xo = -5; xo <= 5; ++xo)
		{
			char c = ((xo == 0 && yo == 0) ? '@' : maze.Char(x + xo, y + yo));
			line += c;
		}
		mapgraph.push_back(line);

	}

	const std::string info_string =
		"In a %s tunnel at %+3ld,%+3ld\n"_f % EnvTypes[room.Env].name % x % -y
		+ "Exits:%s%s%s%s\n\n"_f
		% (CanMoveTo(x + 0, y - 1) ? " north" : "")
		% (CanMoveTo(x + 0, y - +1) ? " south" : "")
		% (CanMoveTo(x - 1, y + 0) ? " west" : "")
		% (CanMoveTo(x + 1, y + 0) ? " east" : "");

		//Print Map and info side by side

		auto m = mapgraph.begin();
		auto b = info_string.begin(), e = info_string.end();
		auto pat = "([^\n]*)\n"_r;
		for (std::smatch res; m != mapgraph.end() || b != e; res = std::smatch{})
		{
			if (b != e) { std::regex_search(b, e, res, pat); b = res[0].second; }
			std::string sa = m != mapgraph.end() ? *m++ : std::string(11, ' ');
			std::string sb = res[1];
			std::cout << "%s | %s\n"_f % sa % sb;
		}


}

static void EatLife(long l)
{
	const char* msg = nullptr;
	if (life >= 80 && life - l < 80)  msg = "You are hungry!\n";
	if (life >= 40 && life - l < 40) msg = "You are starving!\n";
	if (life >= 20 && life - l < 20) msg = "You are close to collapsing\n";
	life -= 1;
	if (msg) { std::cout << msg; }
}



//Character Move Function
static bool TryMoveBy(int xd, int yd)
{
	if (CanMoveTo(x + xd, y + yd) || (!CanMoveTo(x, y + yd) && !CanMoveTo(x + xd, y)))
	{
		std::cout << "You cannot go that way!" << std::endl; 
		return false;
	}
	long burden = 1;
	x += xd;
	y += yd;
	EatLife(burden);

	return true;
}


int main()
{
	std::cout << "Welcome to the Dungeon." << std::endl;
	std::cout << std::endl;

help:
	std::cout << "Available Commands: " << std::endl;
	std::cout << "\tl/look " << std::endl;
	std::cout << "\tn/s/w/e for moving " << std::endl;
	std::cout << "\tquit " << std::endl;
	std::cout << "\thelp " << std::endl;
	std::cout << std::endl;

	std::cout << "You are Starving, You are trying to find enough items to sell in order to buy food " << std::endl;
	std::cout << "BEWARE food is very expensive here... " << std::endl;
	std::cout << std::endl;
	

	//Main Loop
	Look();
	while (life > 0)
	{
		std::cout << "[life:%ld]>"_f % life << std::flush;
		std::string s;
		std::getline(std::cin, s);
		if(!std::cin.good() || s == "quit") break;
		if (s.empty()) continue;

		//Parse with C++11 Regex
		std::smatch res;

		
#define rm std::regex_match


		//Meta Commands
		if (rm(s, R"((?:help|what|\?))"_r)) goto help;

		//Movement Commands
		else if (rm(s, "((go|walk|move) +)?(n|north)"_r)) { if (TryMoveBy(0, -1)) Look(); }
		else if (rm(s, "((go|walk|move) +)?(s|south)"_r)) { if (TryMoveBy(0, 1)) Look(); }
		else if (rm(s, "((go|walk|move) +)?(w|west)"_r)) { if (TryMoveBy(-1, 0)) Look(); }
		else if (rm(s, "((go|walk|move) +)?(e|east)"_r)) { if (TryMoveBy(1, 0)) Look(); }
		else if (rm(s, "((go|walk|move) +)?(nw|northwest"_r)) { if (TryMoveBy(-1, -1)) Look(); }
		else if (rm(s, "((go|walk|move) +)?(ne|northeast)"_r)) { if (TryMoveBy(1, -1)) Look(); }
		else if (rm(s, "((go|walk|move) +)?(sw|southwest)"_r)) { if (TryMoveBy(-1, 1)) Look(); }
		else if (rm(s, "((go|walk|move) +)?(se|southeast)"_r)) { if (TryMoveBy(1, 1)) Look(); }


		//Looking Command and using regex to rec
		else if (rm(s, "(?|l|look)( +around)?"_r)) Look();

		else if (rm(s, R"((?:wear|wield|eq")\b.*)"_r))
			std::cout << "This is not an RPG... yet" << std::endl;
		else if (rm(s, R"(eat\b.*)"_r))
			std::cout << "You have no food yet! Go get some you hobo " << std::endl;
			
		//Unrecognised Command
		else std::cout << "Command not recognised" << std::endl;

	}

	std::cout
		<< "%s\n"_f % (life < 0
			? "You are pulled out from the maze by a supernatural force!"
			: "byebye")
		<< "[life:%ld] Game over\n"_f % life;

}


