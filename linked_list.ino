struct node {
  int value;
  struct node *next;
};

// typedef struct node node_t;

// creates a new list element with given value 
struct node * list_new(int val)
{
  struct node *res = (struct node*) malloc(sizeof(struct node));
  res->value = val;
  res->next = NULL;
  return res;
}

// appends a value to the end of existing list
// returns the added node
// struct node * list_add(int val, struct node *head)
// {
//   struct node *curr;
//   curr = head;

//   // move to end of list
//   while (curr->next)
//   {
//     curr = curr->next;
//   }

//   // add new element
//   curr->next = list_new(val);

//   return curr;
// }

// removes a first value from existing list
// returns the new start of the list, or null in case list size is 1
// struct node * list_rem_head(node * head)
// {
//   struct node *res = head->next;
  
//   free(head);
//   return res;
// }

// removes the last element of the list
void list_rem_tail(node * head)
{
  struct node *curr = head;
  struct node *before_last;
  while (curr->next)
  {
    before_last = curr;
    curr = curr->next;
  }

  free(curr);
  before_last->next = NULL;
}

// returns the length of the list
byte list_len(struct node *head)
{
  byte res = 1;
  struct node *curr = head;
  while (curr->next)
  {
    res++;
    curr = curr->next;
  }
  return res;
}

// append a value to the end of existing list
// if the length of list is bigger(including added element) than provided max 
// value, head of the list is removed
// returns the (new) head of the list
// struct node * list_add_max(int val, byte max, struct node *head)
// {
//   list_add(val, head);

//   // check the length of the list
//   struct node *res = head;
//   if (list_len(head) > max)
//   {
//     res = list_rem_head(head);
//   }
//   return res;
// }


struct node * list_insert_max(int val, byte max, struct node *head)
{
  struct node *ne = list_new(val);
  ne->next = head;

  if (list_len(head) > max)
  {
    list_rem_tail(head);
  }
  return ne;
}
