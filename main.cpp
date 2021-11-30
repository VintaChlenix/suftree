#include <iostream>
#include <vector>
#include <climits>
#include <cassert>
#include <set>
#include <cstring>
#include <string>
#include <unordered_map>

const int CHAR_CODE_LIMIT = 128;
const int INF = INT_MAX / 2; // +бесконечность. Используется как последний символ строки.

struct SuffixNode;
struct SuffixEdge {
	SuffixNode* from, * to;
	int startPos, endPos;
	unsigned char firstChar;
};

// Мы храним рёбра в массиве т.к. это немного упрощает код и увеличивает быстродействие,
// но для экономии памяти можно хранить их в map<char, _suffix_tree_edge>
struct SuffixNode {
	SuffixNode* suffixLink;
	SuffixEdge edges[CHAR_CODE_LIMIT], parentEdge;
	std::unordered_map<unsigned char, SuffixEdge> edges;
	SuffixNode();
};

struct SuffixPos {
	SuffixEdge edge;
	int offset;
};

class SuffixTree {
protected:
	SuffixNode* joker, * root;
private:
	SuffixPos cur;
	SuffixPos GetSuffix(SuffixPos position); // поиск суффикса строки, соотв. текущей позиции
	SuffixNode* SplitEdge(SuffixPos position); // построить новую вершину и суффиксные ссылки
	void DestroyNode(SuffixNode* node);
public:
	std::vector<unsigned char> str; // т.к. в текст буквы добавляются динамически
	SuffixTree();  // инициализация
	void AddLetter(unsigned char ch); // добавить символ в конец текста
	void AddString(std::string& str);
	std::vector<std::pair<int, int> > Find(std::string& str); // Наибольший суффикс каждого префикса параметра str, который 
														  // встречается в исходном тексте (greatest common suffix)
	~SuffixTree();
};

SuffixNode::SuffixNode()
{
	for (int i = 0; i < CHAR_CODE_LIMIT; i++)
	{
		edges[i].from = this;
		edges[i].to = NULL;
		edges[i].startPos = -1;
		edges[i].endPos = 0;
		edges[i].firstChar = i;
	}
}

SuffixTree::SuffixTree()
{
	// инициализация
	joker = new SuffixNode;
	root = new SuffixNode;
	joker->suffixLink = NULL;
	joker->edges[0].to = joker;
	joker->parentEdge = joker->edges[0];
	for (int i = 1; i < CHAR_CODE_LIMIT; i++)
	{
		joker->edges[i].to = root;

	}
	root->parentEdge = joker->edges[1];
	root->suffixLink = joker;
	cur.edge = joker->edges[1];
	cur.offset = 0;
}

void SuffixTree::AddLetter(unsigned char ch) {
	while (true)
	{
		if (cur.edge.endPos == cur.offset)
		{
			// Находимся в узле
			if (cur.edge.to->edges[ch].to == NULL)
			{
			in_a_node:
				// некуда ходить. добавляем лист
				cur.edge.to->edges[ch].to = new SuffixNode;
				cur.edge.to->edges[ch].firstChar = ch;
				cur.edge.to->edges[ch].startPos = str.size();
				cur.edge.to->edges[ch].endPos = INF;
				if (cur.edge.from == joker) break;
				cur = GetSuffix(cur);
			}
			else
			{
				// просто ходим по правильному ребру
				cur.edge = cur.edge.to->edges[ch];
				cur.offset = cur.edge.startPos + 1;
				break;
			}
		}
		else
		{
			// Мы на ребре
			if (str[cur.offset] == ch)
			{
				// идём дальше
				cur.offset++;
				break;
			}
			else
			{
				// создаём вершину...
				SuffixNode* new_node = SplitEdge(cur);
				cur.edge = new_node->parentEdge;
				// и сводим задачу к уже разобранной
				goto in_a_node;
			}
		}
	}
	str.push_back(ch);
}

SuffixPos SuffixTree::GetSuffix(SuffixPos position)
{
	SuffixNode* cur = position.edge.from->suffixLink;
	if (cur == NULL)
	{
		// мы в корне
		return position;
	}
	int cur_offset = position.edge.startPos;
	// опускаемся вниз по дереву ровно столько сколько нужно. одна итерация цикла - спуск по одному ребру.
	while (true)
	{
		SuffixPos res;
		assert(position.edge.endPos - cur_offset != 0);
		SuffixEdge cur_edge = cur->edges[str[cur_offset]];
		cur_offset += cur_edge.endPos - cur_edge.startPos;
		if (cur_offset >= position.offset)
		{
			res.edge = cur_edge;
			res.offset = position.offset - cur_offset + cur_edge.endPos - cur_edge.startPos + cur_edge.startPos;
			return res;
		}
		cur = cur_edge.to;
	}
}

SuffixNode* SuffixTree::SplitEdge(SuffixPos position)
{
	if (position.edge.endPos == position.offset)
		return position.edge.to;
	// разбиваем ребро на 2
	SuffixNode* new_node = new SuffixNode;
	SuffixEdge new_edge;
	new_edge.startPos = position.offset;
	new_edge.endPos = position.edge.endPos;
	new_edge.firstChar = str[position.offset];
	new_edge.from = new_node;
	new_edge.to = position.edge.to;
	new_node->edges[str[position.offset]] = new_edge;
	position.edge.endPos = position.offset;
	position.edge.to = new_node;
	new_node->parentEdge = position.edge.from->edges[position.edge.firstChar] = position.edge;
	// строим суффиксную ссылку. Если вершины в нужном месте нет - применяем ту же процедуру.
	new_node->suffixLink = SplitEdge(GetSuffix(position));
	return new_node;
}

SuffixTree::~SuffixTree()
{
	delete joker;
	DestroyNode(root);
}

void SuffixTree::DestroyNode(SuffixNode* node)
{
	// удалить узел и всех его потомков
	if (node == NULL) return;
	for (int i = 0; i < CHAR_CODE_LIMIT; i++)
	{
		DestroyNode(node->edges[i].to);
	}
	delete node;
}

void SuffixTree::AddString(std::string& str)
{
	for (int i = 0; i < str.size(); i++)
	{
		AddLetter(str[i]);
	}
}

std::vector<std::pair<int, int> > SuffixTree::Find(std::string& str)
{
	std::vector<std::pair<int, int> > res;
	int L = str.length();
	SuffixPos cur;
	cur.edge = joker->edges[1];
	cur.offset = 0;
	int d = 0; // текущая "глубина"
	for (int i = 0; i < L; i++)
	{
		while (true)
		{
			if (cur.edge.endPos == cur.offset)
			{
				// мы в вершине
				if (cur.edge.to->edges[(unsigned char)str[i]].to != NULL)
				{
					cur.edge = cur.edge.to->edges[(unsigned char)str[i]];
					cur.offset = cur.edge.startPos + 1;
					break;
				}
				if (cur.edge.from == joker)
				{
					// добрались до корня. особый случай т.к. cur.edge.start_pos не м.б. cur.offset
										// следовательно мы не можем находиться в джокере
					d--;
					break;
				}
				cur = GetSuffix(cur);
				d--;
			}
			else
			{
				// на ребре
				if ((cur.offset >= (int)this->str.size()) || (this->str[cur.offset] != str[i]))
				{
					cur = GetSuffix(cur);
					d--;
				}
				else
				{
					cur.offset++;
					break;
				}
			}
		}
		d++;
		res.push_back(std::make_pair(d, cur.offset));
	}
	return res;
}

int main(void)
{
	std::string str1, str2;
	SuffixTree tree;
	std::cin >> str1;
	std::cin >> str2;
	tree.AddString(str1);
	std::vector<std::pair<int, int> > v = tree.Find(str2);
	int len = v.size();
	int max = 0;
	for (int i = 0; i < len; i++)
	{
		if (v[i].first > max)
			max = v[i].first;
	}
	std::cout << max << std::endl;
	std::set<std::string> res;
	std::string tmp = "";
	for (int i = 0; i < len; i++)
	{
		if (v[i].first == max) {
			for (int j = max; j > 0; j--) {
				tmp += str1[v[i - j + 1].second - 1];
			}
			res.insert(tmp);
			tmp = "";
		}
	}
	for (auto& it : res) {
		std::cout << it << std::endl;
	}
	return 0;
}
