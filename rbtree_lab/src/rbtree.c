#include "rbtree.h"
#include <stdlib.h>

void _delete_node(rbtree *t, node_t *node);
node_t *_insert(rbtree *t, const key_t key);
node_t *_insert_fixup(rbtree *t, node_t *new_node);
void _rotate_left(rbtree *t, node_t *x);
void _rotate_right(rbtree *t, node_t *x);

void _rb_transplant(rbtree *t, node_t *replacement_node, node_t *substitution_node);
node_t *_find_subtree_min(node_t *x, node_t *nil);
void _rb_delete_fixup(rbtree *t, node_t *x);

rbtree *new_rbtree(void)
{
  // initalize rbtree structure
  rbtree *t = (rbtree *)calloc(1, sizeof(rbtree));

  // initalize NIL node
  node_t *nil = (node_t *)calloc(1, sizeof(node_t));

  // set nils
  t->nil = nil;
  t->root = nil;

  // set NIL node's property
  nil->color = RBTREE_BLACK;
  nil->key = 0;
  nil->parent = nil;
  nil->left = nil;
  nil->right = nil;

  return t;
}

void _delete_node(rbtree *t, node_t *node)
{
  // Base condition: if current node is NIL,
  //                 then it means we are at leaf node.
  if (node == t->nil)
  {
    return;
  }

  // post-order traversal (left->right->root)
  _delete_node(t, node->left);
  _delete_node(t, node->right);

  free(node);
}
void delete_rbtree(rbtree *t)
{
  // we don't need to call _delete_node when root node is NIL node
  // because it means that root has no node for now.
  if (t->root != t->nil)
  {
    _delete_node(t, t->root);
  }

  // so we just free NIL node and rbtree memory.
  free(t->nil);
  free(t);
}

void _rotate_left(rbtree *t, node_t *x)
{
  node_t *y = x->right;
  node_t *beta = y->left;

  x->right = beta;

  if (beta != t->nil) // if y's left node is NIL node, then we don't have to update NIL node's parent.
  {
    y->left->parent = x; // Beta's parent is not conneted, so we update Beta's parent to x.
  }
  y->parent = x->parent;

  if (x->parent == t->nil) // if current x parent node is nil node, we just make y to root.
  {
    t->root = y;
  }
  else if (x->parent->left == x)
  {
    x->parent->left = y;
  }
  else
  {
    x->parent->right = y;
  }

  y->left = x;
  x->parent = y;
}
void _rotate_right(rbtree *t, node_t *y)
{
  node_t *x = y->left;
  node_t *beta = x->right;

  y->left = beta;

  if (beta != t->nil)
  {
    beta->parent = y;
  }

  x->parent = y->parent;

  if (y->parent == t->nil)
  {
    t->root = x;
  }
  else if (y->parent->left == y)
  {
    y->parent->left = x;
  }
  else
  {
    y->parent->right = x;
  }

  x->right = y;
  y->parent = x;
}
node_t *_insert(rbtree *t, const key_t key)
{
  // STEP1: initialize new node
  node_t *new_node = (node_t *)calloc(1, sizeof(node_t));
  new_node->color = RBTREE_RED;
  new_node->key = key;
  new_node->parent = t->nil;
  new_node->left = t->nil;
  new_node->right = t->nil;

  node_t *current = t->root;           // current comparsion node
  node_t *parent_of_new_node = t->nil; // current's parent candidate

  // STEP2: find location to insert node
  while (current != t->nil)
  {
    parent_of_new_node = current; // update parent candidate to current node
    if (new_node->key < current->key)
    {
      current = current->left;
    }
    else
    {
      current = current->right;
    }
  }

  new_node->parent = parent_of_new_node; // link new node's parent

  // STEP3: this procedure does bi-directional connection
  if (parent_of_new_node == t->nil) // if tree is empty,
  {
    t->root = new_node; // make new node to root.
  }
  else if (new_node->key < parent_of_new_node->key)
  {
    parent_of_new_node->left = new_node;
  }
  else
  {
    parent_of_new_node->right = new_node;
  }

  return new_node;
}

node_t *_insert_fixup(rbtree *t, node_t *new_node)
{
  node_t *current = new_node;
  node_t *parent = current->parent;
  node_t *ancestor = current->parent->parent;
  node_t *uncle = NULL;

  // NOTE: The loop should run as long as the parent of the current node is red.
  //       Firstly, we check current parent's node color here.
  //       This check is for case 1.
  while (parent->color == RBTREE_RED)
  {
    // NOTE: Before checking each case,
    //       determine whether the parent is the left or right child of the grandparent.
    if (parent == ancestor->left) // if my parent's location is left of my ancestor,
    {
      uncle = ancestor->right; // set current's uncle node location right to my ancestor node

      // If my uncle node's color is RED, we check case 1.
      if (uncle->color == RBTREE_RED)
      {
        // CASE 1: If both the parent and the uncle of the current node are red,
        //         perform the following operations:
        //         - recolor the parent to BLACK
        //         - recolor the uncle to BLACK
        //         - recolor the ancestor to RED
        //         - move current up to the ancestor
        parent->color = RBTREE_BLACK;
        uncle->color = RBTREE_BLACK;
        ancestor->color = RBTREE_RED;
        current = ancestor;
        parent = current->parent;
        ancestor = parent->parent;
      }
      // If my uncle node's color isn't RED, then we check case 2 -> 3 procedure.
      else
      {
        // CASE 2: 우리는 current의 조상 노드를 기준으로 current의 부모 노드가 어디 있는지 확인했다.
        //         그것을 기준으로 current가 current의 부모 노드 기준으로 어디에 위치해 있는지를 확인하자.
        //         current node가 current의 부모의 오른쪽에 위치해있다는 것은 현재 오른쪽으로 꺾여있는 형태라는 것이다.
        //         이것은 case2 형태에 해당하며, 이것을 case3 형태로 만들기 위해서 rotate left연산을 수행하여 펴준다.
        // NOTE: base point of rotation operation from case2 to case3 is current's parent node
        if (current == parent->right)
        {
          current = parent;
          _rotate_left(t, parent);
        }
        // CASE 3: 직전에 case2의 형태인지 확인했으니, case2의 형태가 아니라면
        //         case3 형태에서 올바른 구조를 만들기 위한 작업을 실시한다.
        //          - set current's parent node's color to BLACK
        //          - set current's ancestor node's color to RED
        //          - rotate right
        // NOTE: base point of rotation operation on case3 is current's ancestor node
        parent->color = RBTREE_BLACK;
        ancestor->color = RBTREE_RED;
        _rotate_right(t, ancestor);
      }
    }
    else // if my parent's location is right of my ancestor,
    {
      uncle = ancestor->left; // we can set my uncle's location to my ancestor's left.

      // if my uncle node's color is RED, we check case 1.
      if (uncle->color == RBTREE_RED)
      {
        // CASE 1: if current's parent node is red and current's uncle node is red,
        //         then do these operation.
        //          - color current's parent into BLACK.
        //          - color current's uncle node into BLACK.
        //          - color current's uncle node into RED.
        //          - move current to current's ancestor.
        parent->color = RBTREE_BLACK;
        uncle->color = RBTREE_BLACK;
        ancestor->color = RBTREE_RED;
        current = ancestor;
        parent = current->parent;
        ancestor = parent->parent;
      }
      else // if my uncle node's color isn't RED, then we check case 2 -> 3 level.
      {
        // CASE 2: 우리는 current의 조상 노드를 기준으로 current의 부모 노드가 어디 있는지 확인했다.
        //         그것을 기준으로 current가 current의 부모 노드 기준으로 어디에 위치해 있는지를 확인하자.
        //         current node가 current의 부모 노드의 왼쪽에 위치해있다는 것은 현재 왼쪽으로 꺾여있는 형태라는 것이다.
        //         이것은 case2 형태에 해당하며, 이것을 case3 형태로 만들기 위해서 rotate right연산을 수행하여 펴준다.
        // NOTE: base point of rotation operation from case2 to case3 is current's parent node
        if (current == parent->left)
        {
          current = parent;
          _rotate_right(t, parent);
        }
        // CASE 3: 직전에 case2의 형태인지 확인했으니, case2의 형태가 아니라면
        //         case3 형태에서 올바른 구조를 만들기 위한 작업을 실시한다.
        //          - set current's parent node's color to BLACK
        //          - set current's ancestor node's color to RED
        //          - rotate left
        // NOTE: base point of rotation operation on case3 is current's ancestor node
        parent->color = RBTREE_BLACK;
        ancestor->color = RBTREE_RED;
        _rotate_left(t, ancestor);
      }
    }
  }

  // color root node to black
  t->root->color = RBTREE_BLACK;

  return t->root;
}
node_t *rbtree_insert(rbtree *t, const key_t key)
{
  // STEP: insert node
  node_t *new_node = _insert(t, key);

  // STEP: fix to Red-Black tree structure
  node_t *root = _insert_fixup(t, new_node);

  return root;
}

node_t *rbtree_find(const rbtree *t, const key_t key)
{
  node_t *current = t->root;
  while (current != t->nil)
  {
    if (current->key < key)
    {
      current = current->right;
    }
    else if (current->key > key)
    {
      current = current->left;
    }
    else
    {
      return current;
    }
  }
  return NULL;
}

node_t *rbtree_min(const rbtree *t)
{
  node_t *current = t->root;
  if (current == t->nil)
    return t->nil;

  while (current->left != t->nil)
  {
    current = current->left;
  }
  return current;
}

node_t *rbtree_max(const rbtree *t)
{
  node_t *current = t->root;
  if (current == t->nil)
    return t->nil;

  while (current->right != t->nil)
  {
    current = current->right;
  }
  return current;
}

void _rb_transplant(rbtree *t, node_t *replacement_node, node_t *substitution_node)
{
  if (replacement_node->parent == t->nil)
  {
    t->root = substitution_node;
  }
  else if (replacement_node->parent->left == replacement_node)
  {
    replacement_node->parent->left = substitution_node;
  }
  else
  {
    replacement_node->parent->right = substitution_node;
  }
  substitution_node->parent = replacement_node->parent;
}
node_t *_find_subtree_min(node_t *right_subtree, node_t *nil)
{
  node_t *current = right_subtree;

  while (current->left != nil)
  {
    current = current->left;
  }
  return current;
}
void _rb_delete_fixup(rbtree *t, node_t *curr)
{
  node_t *brother = NULL;
  while (curr != t->root && curr->color == RBTREE_BLACK)
  {
    // NOTE: check current location
    if (curr == curr->parent->left)
    {
      brother = curr->parent->right; // set curr's brother

      // CASE 1: 내 형제의 색깔이 RED일 경우
      if (brother->color == RBTREE_RED)
      {
        brother->color = RBTREE_BLACK;
        curr->parent->color = RBTREE_RED;
        _rotate_left(t, curr->parent);
        brother = curr->parent->right; // set new brother of curr
      }

      // CASE 2: 내 형제의 자식 노드 둘다 색깔이 BLACK일 경우
      if (brother->left->color == RBTREE_BLACK &&
          brother->right->color == RBTREE_BLACK)
      {
        // NOTE: 흡성대법 이후 다시 while문 체크하러 감
        brother->color = RBTREE_RED;
        curr = curr->parent;
      }
      else
      {
        // CASE 3: 내 형제의 자식 중 나와 가까운 쪽 자식 색상이 RED이고, 먼 쪽이 BLACK일 경우
        if (brother->left->color == RBTREE_RED &&
            brother->right->color == RBTREE_BLACK)
        {
          // NOTE: brother의 왼쪽 자식(나와 가까운 쪽 자식)의 색깔을 BLACK
          //       brother의 색상을 RED로
          brother->left->color = RBTREE_BLACK;
          brother->color = RBTREE_RED;
          _rotate_right(t, brother);
          brother = curr->parent->right; // set new brother of curr
        }
        // CASE 4: 내 형제의 자식 중 나와 먼 쪽이 RED일 경우
        // NOTE: brother와 brother의 부모와의 색상 교환이 이루어진다
        //        - broter를 brother parent(curr의 parent 정보로)의 색으로 칠함
        //        - brother parent는 기존 brother의 색상이었던 BLACK으로 칠해준다
        brother->color = curr->parent->color;
        curr->parent->color = RBTREE_BLACK;
        brother->right->color = RBTREE_BLACK;
        _rotate_left(t, curr->parent);
        curr = t->root; // double black 해결
      }
    }
    else
    {
      brother = curr->parent->left;

      // CASE 1: 내 형제의 색깔이 RED일 경우
      if (brother->color == RBTREE_RED)
      {
        brother->color = RBTREE_BLACK;
        curr->parent->color = RBTREE_RED;
        _rotate_right(t, curr->parent);
        brother = curr->parent->left; // set new brother of curr
      }

      // CASE 2: 내 형제의 자식 노드 둘다 색깔이 BLACK일 경우
      if (brother->left->color == RBTREE_BLACK &&
          brother->right->color == RBTREE_BLACK)
      {
        // NOTE: 흡성대법 이후 다시 while문 체크하러 감
        brother->color = RBTREE_RED;
        curr = curr->parent;
      }
      else
      {
        // CASE 3: 내 형제의 자식 중 나와 가까운 쪽 자식 색상이 RED이고, 먼 쪽이 BLACK일 경우
        if (brother->left->color == RBTREE_BLACK &&
            brother->right->color == RBTREE_RED)
        {
          // STEP: brother와 brother의 부모와의 색상 교환이 이루어진다
          //       - brother의 오른쪽 자식(나와 가까운 쪽 자식)의 색깔을 BLACK
          //       - brother의 색상을 RED로
          brother->right->color = RBTREE_BLACK;
          brother->color = RBTREE_RED;
          _rotate_left(t, brother);
          brother = curr->parent->left; // set new brother of curr
        }
        // CASE 4: 내 형제의 자식 중 나와 먼 쪽이 RED일 경우
        // STEP: brother와 brother의 부모와의 색상 교환이 이루어진다
        //       - broter를 brother parent(curr의 parent 정보로)의 색으로 칠함
        //       - brother parent는 기존 brother의 색상이었던 BLACK으로 칠해준다
        brother->color = curr->parent->color;
        curr->parent->color = RBTREE_BLACK;
        brother->left->color = RBTREE_BLACK;
        _rotate_right(t, curr->parent);
        curr = t->root; // double black 해결
      }
    }
  }
  curr->color = RBTREE_BLACK;
}
int rbtree_erase(rbtree *t, node_t *p)
{
  node_t *target_node_to_delete = NULL;
  key_t removed_node_original_color = RBTREE_BLACK;

  node_t *fixup_start_node = NULL;
  node_t *node_actually_removed = NULL;

  // STEP 1: find target node
  target_node_to_delete = rbtree_find(t, p->key);

  // ! if we cannot find target node, then there's nothing to erase.
  if (target_node_to_delete == NULL)
  {
    return 0;
  }

  // NOTE: Save color of node that will actually be removed from the tree
  node_actually_removed = target_node_to_delete;
  removed_node_original_color = target_node_to_delete->color;

  // CASE 1: target node has no children
  if (target_node_to_delete->left == t->nil &&
      target_node_to_delete->right == t->nil)
  {
    fixup_start_node = t->nil;
    _rb_transplant(t, target_node_to_delete, t->nil);
  }

  // CASE 2: only right child exists
  else if (target_node_to_delete->left == t->nil)
  {
    fixup_start_node = target_node_to_delete->right;
    _rb_transplant(t, target_node_to_delete, target_node_to_delete->right);
  }

  // CASE 3: only left child exists
  else if (target_node_to_delete->right == t->nil)
  {
    fixup_start_node = target_node_to_delete->left;
    _rb_transplant(t, target_node_to_delete, target_node_to_delete->left);
  }

  // CASE 4: both children exist
  else
  {
    node_actually_removed = _find_subtree_min(target_node_to_delete->right, t->nil);

    removed_node_original_color = node_actually_removed->color;

    fixup_start_node = node_actually_removed->right;

    if (node_actually_removed != target_node_to_delete->right)
    {
      _rb_transplant(t, node_actually_removed, node_actually_removed->right);
      node_actually_removed->right = target_node_to_delete->right;
      node_actually_removed->right->parent = node_actually_removed;
    }
    else
    {
      fixup_start_node->parent = node_actually_removed;
    }

    _rb_transplant(t, target_node_to_delete, node_actually_removed);
    node_actually_removed->left = target_node_to_delete->left;
    node_actually_removed->left->parent = node_actually_removed;
    node_actually_removed->color = target_node_to_delete->color;
  }

  if (removed_node_original_color == RBTREE_BLACK)
  {
    _rb_delete_fixup(t, fixup_start_node);
  }

  free(target_node_to_delete);

  return 0;
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n)
{
  // TODO: implement to_array
  return 0;
}