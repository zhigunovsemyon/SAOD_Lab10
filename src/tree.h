#ifndef TREE_H_
#define TREE_H_


// Узел дерева
typedef struct Node {
    int key;
    struct Node *left;
    struct Node *right;
} Node;

// Структура дерева
typedef struct ScapegoatTree {
    Node *root;
    int size; // Количество узлов в дереве
    int maxSize; // Максимальный размер дерева для отслеживания необходимости ребалансировки
} ScapegoatTree;

// Инициализация дерева
ScapegoatTree * Tree_create();

// Освобождение дерева
void Tree_free(ScapegoatTree ** tree);

// Поиск нужной ячейки
Node const * Tree_find(ScapegoatTree const * tree, int key);

//Вставка ключа в дерево
bool Tree_insert(ScapegoatTree * tree, int key);

#endif // !TREE_H_
