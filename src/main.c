#include "tree.h"
#include <stdio.h>

static void Tree_print_(Node const * n)
{
	if (!n)
		return;

	Tree_print_(n->left);

	printf("%d\n", n->key);

	Tree_print_(n->right);
}


static void Tree_print(ScapegoatTree const * tree)
{
	Tree_print_(tree->root);
}

int main()
{
	int a[] = {1, 4, 7, 2, 3, -8, 0};
	int a_size = sizeof(a) / sizeof(*a);

	ScapegoatTree * tree = Tree_create();
	if (!tree){
		perror("Tree_create(): ");
		return -1;
	}

	for (int i = 0; i < a_size; i++) {
		Tree_insert(tree, a[i]);
		printf("Inserted %d,\tsize: %d\n", a[i], tree->size);
	}

	Tree_print(tree);

	int searchKey;
	fputs("Введите значение: ",stdout);
	if (1 != scanf("%d", &searchKey)){
		Tree_free(&tree);
		return 0;
	}

	// Поиск
	Node const * result = Tree_find(tree, searchKey);
	printf("Поиск %d: %s\n", searchKey,
	       result ? "Найден" : "Не найден");

	// Очистка
	Tree_free(&tree);

	return 0;
}
