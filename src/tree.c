#include "tree.h"
#include <assert.h>
#include <malloc.h>  /*malloc(), free()*/
#include <stdbool.h> /*bool*/
#include <string.h>  /*memcpy()*/

typedef char byte;

static struct TreeNode ** TreeRightmostNode_(struct TreeNode ** const ogNode)
{
	assert(ogNode != NULL);
	assert(*ogNode != NULL);

	return ((*ogNode)->r == NULL) ? ogNode
				      : TreeRightmostNode_(&(*ogNode)->r);
}

static struct TreeNode ** TreeLeftmostNode_(struct TreeNode ** const ogNode)
{
	assert(ogNode != NULL);
	assert(*ogNode != NULL);

	return ((*ogNode)->l == NULL) ? ogNode
				      : TreeLeftmostNode_(&(*ogNode)->l);
}

/*Очистка дерева обратным обходом LRN*/
static void TreeFree_(struct TreeNode ** node)
{
	/*Если передана пустая ячейка*/
	if ((*node) == NULL)
		return;

	/*Очистка в ветвях*/
	TreeFree_(&((*node)->l));
	TreeFree_(&((*node)->r));

	free(*node);
	*node = NULL;
	return;
}

/*Инициализация дерева. Принимает функцию распределения и размер данных*/
Tree * TreeInit(size_t esize, compar_fn compar)
{
	if (esize == 0 || compar == NULL)
		return NULL;

	Tree * pTree = (Tree *)malloc(sizeof(Tree));
	if (!pTree)
		return NULL;

	pTree->esize = esize;
	pTree->compar = compar;
	pTree->root = NULL;

	return pTree;
}

static struct TreeNode *
NodeNew_(struct TreeNode * p, void * const src, size_t esize)
{
	struct TreeNode * pNode =
		(struct TreeNode *)malloc(sizeof(struct TreeNode) + esize);
	if (pNode == NULL)
		return pNode /*NULL*/;

	pNode->p = p;
	pNode->r = pNode->l = NULL;
	pNode->data = pNode + 1;

	memcpy(pNode->data, src, esize);
	return pNode;
}

static bool TreeInsert_(struct TreeNode ** pNode,
			struct TreeNode * pParentNode,
			void * const src,
			compar_fn compar,
			size_t esize)
{
	if (*pNode == NULL) {
		*pNode = NodeNew_(pParentNode, src, esize);
		return (*pNode) ? true : false; /*true при удачной записи, false
					    при ошибке malloc*/
	}
	/*Здесь и далее pNode не указывает на null-указатель*/
	assert(*pNode != NULL);

	int cmp_res = compar((*pNode)->data, src);

	/*Если под pNode находится элемент, равный источнику*/
	if (cmp_res == 0) {
		memcpy((*pNode)->data, src, esize); /*он перезаписывается*/
		return 1;
	}
	/*else*/
	return (cmp_res > 0)
		       ? TreeInsert_(&(*pNode)->l, *pNode, src, compar, esize)
		       : TreeInsert_(&(*pNode)->r, *pNode, src, compar, esize);
}

int TreeInsertArray(Tree * pTree, void * const data, size_t arrlen)
{
	int count = 0;
	for (size_t i = 0; i < arrlen; ++i) {
		if (TreeInsert(pTree, (byte *)data + i * pTree->esize))
			++count;
		else
			break;
	}
	return count;
}

/*Вставка элемента данных*/
int TreeInsert(Tree * pTree, void * const src)
{
	return TreeInsert_(&pTree->root, NULL, src, pTree->compar,
			   pTree->esize);
}

/*Прямой NLR поиск по дереву*/
static struct TreeNode **
TreeLocate_(struct TreeNode ** pNode, void * const key, compar_fn compar)
{
	if (*pNode == NULL)
		return pNode;

	/*Здесь и далее pNode не указывает на null-указатель*/
	assert(*pNode != NULL);

	int cmp_res = compar((*pNode)->data, key);

	/*Если под pNode находится элемент, равный источнику*/
	if (cmp_res == 0) {
		return pNode;
	}
	/*else*/
	return (cmp_res > 0) ? TreeLocate_(&(*pNode)->l, key, compar)
			     : TreeLocate_(&(*pNode)->r, key, compar);
}

static void NodeRemove_(struct TreeNode ** pNode)
{
	assert(*pNode != NULL);
	/*Сохранение указателя на удаляемый элемент*/
	struct TreeNode * rem = *pNode;
	/*Элемент на замену*/
	struct TreeNode * Replacement = NULL;

	/*Если у ноды есть левый потомок, самый правый элемент в левой половине
	 * ставится на место ноды*/
	if (rem->l != NULL) {
		/*Извлечение самого правого элемента*/
		struct TreeNode ** pRightmost = TreeRightmostNode_(&rem->l);
		assert((*pRightmost)->p != NULL);
		Replacement = *pRightmost;
		*pRightmost = Replacement->l; // Сносит старый указатель на

		/*Перестановка указателей в извлечённом элементе*/
		Replacement->p = rem->p;
		Replacement->l = rem->l;
		Replacement->r = rem->r;

		/*Подстановка указателя в другом потомке на
		 * новую родительскую ноду*/
		if (rem->r != NULL)
			rem->r->p = Replacement;
	} else if (rem->r != NULL) {
		/*Если у ноды нет левого потомка, но есть правый, самый левый
		 * элемент в правой половине ставится на место ноды*/

		/*Извлечение самого левого элемента*/
		struct TreeNode ** pLeftmost = TreeLeftmostNode_(&rem->r);
		assert((*pLeftmost)->p != NULL);
		Replacement = *pLeftmost;
		*pLeftmost = Replacement->r;

		/*Перестановка указателей в извлечённом элементе*/
		Replacement->p = rem->p;
		Replacement->l = rem->l;
		Replacement->r = rem->r;

		/*Подстановка указателя в другом потомке на
		 * новую родительскую ноду*/
		if (rem->l != NULL)
			rem->l->p = Replacement;
	}
	/*Если нода является листом, указатель будет перезаисан NULLом */

	/*Подстановка на место удаляемого*/
	*pNode = Replacement;
	/*Очистка*/
	free(rem);
}

/*Удаление элемента с переданным ключом*/
int TreeRemove(Tree * pTree, void * const key)
{
	struct TreeNode ** pNode =
		TreeLocate_(&pTree->root, key, pTree->compar);
	if (*pNode == NULL)
		return 0;

	assert(*pNode != NULL);
	NodeRemove_(pNode);
	return 1;
}

int TreeBelongs(Tree * pTree, void * const key)
{
	struct TreeNode ** pNode =
		TreeLocate_(&pTree->root, key, pTree->compar);
	return (*pNode != NULL) ? 1 : 0;
}

/*Копирование элемента по ключу key из дерева в dest*/
int TreeCopy(Tree * pTree, void * const key, void * dest)
{
	struct TreeNode ** pNode =
		TreeLocate_(&pTree->root, key, pTree->compar);
	if (*pNode == NULL)
		return 0;
	/*else*/
	memcpy(dest, (*pNode)->data, pTree->esize);
	return 1;
}

/*Уничтожение дерева*/
void TreeFree(Tree * pTree)
{
	TreeFree_(&(pTree->root));
	free(pTree);
}
