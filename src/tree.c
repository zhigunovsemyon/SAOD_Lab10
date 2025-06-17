#include "tree.h"
#include <malloc.h>
#include <math.h>
#include <stddef.h>

static constexpr double ALPHA = .8;

// Создание нового узла. Возврат NULL при неудаче
static Node * createNode(int key)
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
static int size(Node const * node)
{
	if (!node)
		return 0;
	return 1 + size(node->left) + size(node->right);
}

// Подсчёт высоты дерева
static int Node_height(Node const * node)
{
	if (!node)
		return 0;
	int leftHeight = Node_height(node->left);
	int rightHeight = Node_height(node->right);

	return 1 + (leftHeight > rightHeight ? leftHeight : rightHeight);
}

// Проверка необходимости ребалансировки
static int isUnbalanced(ScapegoatTree const * tree, int depth)
{
	// log от n по основанию 1/alpha
	double lg = log(tree->size) / log(1 / ALPHA);
	return depth > floor(lg);
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

	int index = 0; //Вспомогательный индекс для storeNodes
	storeNodes(node, nodes, &index);
	Node * newRoot = buildBalanced(nodes, 0, subtreeSize - 1);
	free(nodes);
	return newRoot;
}

// Поиск scapegoat (узла, вызывающего несбалансированность).
// Возвращает NULL в сбалансированном дереве.
static Node *
findScapegoat(Node * node, Node * inserted, int * depth, ScapegoatTree * tree)
{
	if (!node)
		return NULL;

	(*depth)++;
	if (node == inserted && isUnbalanced(tree, *depth)) {
		return node;
	}

	if (inserted->key < node->key) { // Поиск в левой ветви
		Node * scapegoat =
			findScapegoat(node->left, inserted, depth, tree);
		if (scapegoat)
			return scapegoat;
		if (size(node->left) > ALPHA * size(node))
			return node;
	} else { // Поиск в правой ветви
		Node * scapegoat =
			findScapegoat(node->right, inserted, depth, tree);
		if (scapegoat)
			return scapegoat;
		if (size(node->right) > ALPHA * size(node))
			return node;
	}
	return NULL;
}

// Вставка узла. Возврат NULL при неудаче.
static Node *
Node_insert(Node * node, int key, ScapegoatTree * tree, Node ** insertedNode)
{
	if (!node) {
		tree->size++;
		Node * new_node = createNode(key);
		*insertedNode = new_node; // может выставить null
		return *insertedNode;
	}

	if (key < node->key) {
		// Попытка вставить в левой ветви
		node->left = Node_insert(node->left, key, tree, insertedNode);
	} else if (key > node->key) {
		// Попытка вставить в правой ветви
		node->right = Node_insert(node->right, key, tree, insertedNode);
	}

	// Возвращение этого узла
	return node;
}

// Поиск ключа
static Node const * Node_find_(Node const * node, int key)
{
	if (!node || node->key == key)
		return node;

	if (key < node->key)
		return Node_find_(node->left, key);

	return Node_find_(node->right, key);
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
	tree->root = Node_insert(tree->root, key, tree, &insertedNode);
	if (!insertedNode)
		return true;

	if (isUnbalanced(tree, Node_height(tree->root))) {
		int depth = 0;
		Node * scapegoat =
			findScapegoat(tree->root, insertedNode, &depth, tree);
		if (!scapegoat) // дерево сбалансировано
			return false;

		int subtreeSize = size(scapegoat);
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
	}

	if (tree->size > tree->maxSize)
		tree->maxSize = tree->size;
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

