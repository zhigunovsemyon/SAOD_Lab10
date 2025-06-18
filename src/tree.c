#include "tree.h"
#include <malloc.h>
#include <math.h>
#include <stddef.h>

static constexpr double ALPHA = .6;

// Проверка необходимости ребалансировки (глубина > log(1/alpha) от размера)
static bool isUnbalanced(int tree_size, int depth)
{
	// log от n по основанию 1/alpha
	double lg = log(tree_size) / log(1 / ALPHA);
	return depth > floor(lg);
}

// Создание нового узла. Возврат NULL при неудаче
static Node * Node_create_(int key)
{
	Node * node = (Node *)malloc(sizeof(Node));
	if (!node)
		return NULL;

	node->key = key;
	node->left = NULL;
	node->right = NULL;
	return node;
}

// Подсчёт размера поддерева
static int Node_size_(Node const * node)
{
	if (!node)
		return 0;
	return 1 + Node_size_(node->left) + Node_size_(node->right);
}

// Подсчёт высоты дерева
static int Node_height_(Node const * node)
{
	if (!node)
		return 0;
	int leftHeight = Node_height_(node->left);
	int rightHeight = Node_height_(node->right);
	int biggerHeight =
		(leftHeight > rightHeight ? leftHeight : rightHeight);

	return 1 + biggerHeight;
}

// Сбор узлов поддерева в массив для перестройки
static void storeNodes(Node * node, Node ** array, int * index)
{
	if (!node)
		return;

	storeNodes(node->left, array, index);
	array[(*index)++] = node;
	storeNodes(node->right, array, index);
}

// Построение сбалансированного поддерева из массива узлов
static Node * buildBalanced(Node ** array, int start, int end)
{
	if (start > end)
		return NULL;

	int mid = (start + end) / 2;
	Node * node = array[mid];

	node->left = buildBalanced(array, start, mid - 1);
	node->right = buildBalanced(array, mid + 1, end);

	return node;
}

// Ребалансировка поддерева. Возврат NULL при
// неудаче создания вспомогательного массива
static Node * rebuildSubtree(Node * node, int subtreeSize)
{
	Node ** nodes = (Node **)malloc((size_t)subtreeSize * sizeof(Node *));
	if (!nodes)
		return NULL;

	int index = 0; // Вспомогательный индекс для storeNodes
	storeNodes(node, nodes, &index);
	Node * newRoot = buildBalanced(nodes, 0, subtreeSize - 1);
	free(nodes);
	return newRoot;
}

// Поиск scapegoat (узла, вызывающего несбалансированность).
// Возвращает NULL в сбалансированном дереве.
static Node *
findScapegoat(Node * node, Node * inserted, int * depth, int tree_size)
{
	if (!node)
		return NULL;
	(*depth)++;

	if (node == inserted)
		return node;
	if (isUnbalanced(tree_size, *depth))
		return node;

	// Поиск в левой ветви
	if (inserted->key < node->key) {
		Node * scapegoat =
			findScapegoat(node->left, inserted, depth, tree_size);
		if (scapegoat)
			return scapegoat;
		if (Node_size_(node->left) > ALPHA * Node_size_(node))
			return node;
	}
	// Поиск в правой ветви
	else {
		Node * scapegoat =
			findScapegoat(node->right, inserted, depth, tree_size);
		if (scapegoat)
			return scapegoat;
		if (Node_size_(node->right) > ALPHA * Node_size_(node))
			return node;
	}
	return NULL;
}

// Вставка узла. Возврат NULL при неудаче.
static Node * Node_insert(Node * node, int key, Node ** insertedNode)
{
	if (!node) {
		Node * new_node = Node_create_(key);
		*insertedNode = new_node; // может выставить null
		return *insertedNode;
		// После возвращения нового узла,
		// дерево "разворачивается" (см. ниже), пока не вернёт корень
	}

	if (key < node->key) {
		// Попытка вставить в левой ветви
		node->left = Node_insert(node->left, key, insertedNode);
	} else if (key > node->key) {
		// Попытка вставить в правой ветви
		node->right = Node_insert(node->right, key, insertedNode);
	}

	// Возвращение этого узла
	return node;
}

// Поиск ключа
static Node const * Node_find_(Node const * node, int key)
{
	if (!node)
		return node;
	if (node->key == key)
		return node;

	return (key < node->key) ? Node_find_(node->left, key)
				 : Node_find_(node->right, key);
}

// Освобождение памяти
static void Node_free(Node * node)
{
	if (!node)
		return;

	Node_free(node->left);
	Node_free(node->right);
	free(node);
}

void Tree_free(ScapegoatTree ** tree)
{
	Node_free((*tree)->root);
	free(*tree);
	*tree = NULL;
}

// Основная функция вставки с ребалансировкой. 1 при неудаче
bool Tree_insert(ScapegoatTree * tree, int key)
{
	Node * insertedNode = NULL;
	tree->root = Node_insert(tree->root, key, &insertedNode);
	if (!insertedNode)
		return true;

	tree->size++;
	if (tree->size > tree->maxSize)
		tree->maxSize = tree->size;

	bool const isBalanced =
		!isUnbalanced(tree->size, Node_height_(tree->root));
	if (isBalanced)
		return false;

	int depth = 0;
	Node * scapegoat =
		findScapegoat(tree->root, insertedNode, &depth, tree->size);
	if (!scapegoat) // дерево сбалансировано
		return false;

	int subtreeSize = Node_size_(scapegoat);
	Node * parent = tree->root;
	Node * newSubtree = rebuildSubtree(scapegoat, subtreeSize);
	if (!newSubtree) // при неудаче rebuildSubtree
		return true;

	// Найти родителя scapegoat и заменить поддерево
	if (parent == scapegoat) {
		tree->root = newSubtree;
	} else {
		while (parent->left != scapegoat &&
		       parent->right != scapegoat) {
			if (scapegoat->key < parent->key) {
				parent = parent->left;
			} else {
				parent = parent->right;
			}
		}
		if (parent->left == scapegoat) {
			parent->left = newSubtree;
		} else {
			parent->right = newSubtree;
		}
	}

	return false;
}

Node const * Tree_find(ScapegoatTree const * tree, int key)
{
	return Node_find_(tree->root, key);
}

// Инициализация дерева
ScapegoatTree * Tree_create()
{
	ScapegoatTree * tree = (ScapegoatTree *)malloc(sizeof(ScapegoatTree));
	if (!tree)
		return NULL;

	tree->root = NULL;
	tree->size = 0;
	tree->maxSize = 0;
	return tree;
}
