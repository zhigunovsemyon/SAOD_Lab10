#ifndef TREE_H_
#define TREE_H_
#include <stddef.h> /*size_t*/

/*Функция сортировки*/
typedef int (*compar_fn)(void const *, void const *);

typedef struct {
	compar_fn compar;	/*Функция сортировки*/
	size_t esize;		/*Размер данных в элементе*/
	struct TreeNode * root; /*Указатель на корень*/
} Tree;

struct TreeNode {
	struct TreeNode *l, *r, *p;
	void * data;
};

/*Инициализация дерева. Принимает функцию распределения и размер данных*/
Tree * TreeInit(size_t esize, compar_fn);

/*Уничтожение дерева*/
void TreeFree(Tree *);

/*Вставка элемента данных. Возврат true при удачной вставке, false при неудаче*/
int TreeInsert(Tree *, void * const src);

/*Вставка массива элементов данных. Возврат количества вставленных элементов */
int TreeInsertArray(Tree *, void * const data, size_t arrlen);

/*Удаление элемента с переданным ключом. Возврат 0 при неудаче, 1 при удаче*/
int TreeRemove(Tree *, void * const key);

/*Копирование элемента по ключу key из дерева в dest.
 * Возвращает 1 при удаче, 0 при неудаче*/
int TreeCopy(Tree *, void * const key, void * dest);

/*Проверка элемента по ключу key из дерева на наличие.
 * Возвращает 1 при удаче, 0 при неудаче*/
int TreeBelongs(Tree *, void * const key);

#endif // !TREE_H_
