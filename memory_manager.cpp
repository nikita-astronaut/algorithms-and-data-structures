#include <algorithm>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <stdexcept>
#include <vector>
#include <utility>

template <class T, class Compare = std::less<T>>
class Heap {
 public:
  using IndexChangeObserver =
      std::function<void(const T& element, size_t new_element_index)>;

  static constexpr size_t kNullIndex = static_cast<size_t>(-1);

  explicit Heap(
      Compare compare = Compare(),
      IndexChangeObserver index_change_observer = IndexChangeObserver()) :
    compare_(compare), index_change_observer_(index_change_observer) {}

  size_t push(const T& value) {
    elements_.push_back(value);
    NotifyIndexChange(elements_.back(), elements_.size() - 1);
    size_t final_index = SiftUp(size() - 1);
    return final_index;
  }

  void erase(size_t index) {
    SwapElements(index, size() - 1);
    elements_.pop_back();
    SiftUp(index);
    SiftDown(index);
  }

  const T& top() const {
    return elements_.front();
  }
  
  void pop() {
    SwapElements(0, size() - 1);
    elements_.pop_back();
    SiftDown(0);
  }

  size_t size() const {
    return elements_.size();
  }
  bool empty() const {
    return elements_.size() == 0;
  }

 private:
  IndexChangeObserver index_change_observer_;
  Compare compare_;
  std::vector<T> elements_;

  size_t Parent(size_t index) const {
    return (index - 1) / 2; 
  }

  size_t LeftSon(size_t index) const {
    return 2 * index + 1;
  }

  size_t RightSon(size_t index) const {
    return 2 * index + 2;
  }

  bool CompareElements(size_t first_index, size_t second_index) const {
    return compare_(elements_[first_index], elements_[second_index]);
  }

  void NotifyIndexChange(const T& element, size_t new_element_index) {
    index_change_observer_(element, new_element_index);
  }

  void SwapElements(size_t first_index, size_t second_index) {
    std::iter_swap(elements_.begin() + first_index, elements_.begin() + second_index);
    NotifyIndexChange(elements_[second_index], second_index);
    NotifyIndexChange(elements_[first_index], first_index);
  }

  size_t SiftUp(size_t index) {
    while (index > 0) {
      if (!CompareElements(index, Parent(index))) {
        SwapElements(index, Parent(index));
        index = Parent(index);
      } else {
        break;
      }
    }
    return index;
  }

  void SiftDown(size_t index) {
    while (LeftSon(index) < size() && RightSon(index) < size()) {
      if (CompareElements(index, LeftSon(index)) || CompareElements(index, RightSon(index))) {
        if (CompareElements(LeftSon(index), RightSon(index))) {
          SwapElements(RightSon(index), index);
          index = RightSon(index);
        } else {
          SwapElements(LeftSon(index), index);
          index = LeftSon(index);
        }
      } else {
        break;
      }
    }

    if (LeftSon(index) < size()) {
      if (CompareElements(index, LeftSon(index))) {
        SwapElements(LeftSon(index), index);
        index = LeftSon(index);
      }
    }
  }
};


struct MemorySegment {
  int left;
  int right;
  size_t heap_index;

  MemorySegment(int init_left, int init_right) {
    left = init_left;
    right = init_right;
  }

  size_t Size() const {
    return right - left;
  }
};

using MemorySegmentIterator = std::list<MemorySegment>::iterator;
using MemorySegmentConstIterator = std::list<MemorySegment>::const_iterator;


struct MemorySegmentSizeCompare {
  bool operator() (const MemorySegmentIterator& first,
                   const MemorySegmentIterator& second) const {
    if (first->Size() < second->Size()) {
      return true;
    } 

    if (first->Size() == second->Size() && first->left > second->left) {
      return true;
    }
    
    return false;
  }
};


using MemorySegmentHeap = 
    Heap<MemorySegmentIterator, MemorySegmentSizeCompare>;


struct MemorySegmentsHeapObserver {
  void operator() (MemorySegmentIterator segment, size_t new_index) const {
    segment->heap_index = new_index;
  }
};

class MemoryManager {
 public:
  using Iterator = MemorySegmentIterator;
  using ConstIterator = MemorySegmentConstIterator;

  explicit MemoryManager(size_t memory_size) :
    free_memory_segments_(MemorySegmentSizeCompare(), MemorySegmentsHeapObserver()) {
    MemorySegment initial_memory_segment = MemorySegment(0, memory_size);
    memory_segments_.push_back(initial_memory_segment);
    free_memory_segments_.push(memory_segments_.begin());
  }

  Iterator Allocate(size_t size) {
    if (free_memory_segments_.empty()) {
      return memory_segments_.end();
    }

    if (size > free_memory_segments_.top()->Size()) {
      return memory_segments_.end();
    } else {
      Iterator top_element_iterator = free_memory_segments_.top();
      free_memory_segments_.pop();
      
      MemorySegment allocated_segment = MemorySegment(top_element_iterator->left, 
        top_element_iterator->left + size);
      allocated_segment.heap_index = MemorySegmentHeap::kNullIndex;

      MemorySegment free_segment = MemorySegment(top_element_iterator->left + size, 
        top_element_iterator->right);
      Iterator following_iterator = memory_segments_.erase(top_element_iterator);
      memory_segments_.insert(following_iterator, allocated_segment);
      
      Iterator allocated_segment_iterator;
      if (free_segment.Size() == 0) {
        allocated_segment_iterator = --following_iterator;
      } else {
        memory_segments_.insert(following_iterator, free_segment);

        Iterator free_segment_iterator = --following_iterator;
        allocated_segment_iterator = --following_iterator;
        free_memory_segments_.push(free_segment_iterator);
      }  
      return allocated_segment_iterator;
    }
  }
  /* это не комментарий, а вопрос, поэтому напишу по-русски */
  /* сначала здесь после освобождения куска происходит обхединение областей памяти. сначала я пользовался 
  ниженаписанной функцией AppendIfFree. Работало всё таким образом: сначала кусок освобождался и просто возвращался
  обратно в лист и одновременно клался в кучу. А затем я натравливал функцию AppendIfFree на пары с левым и правым
  соседями. Они, соответственно, если были свободны, вынимались из листа и кучи вместе с новым элементом, о
  объединялись и клались обратно (мне кажется, так и предполагалось делать).
  Однако такой способ не проходил на последних data-сетах (тесты 90-91), при этом проходил на дата-сетах
  того же размера за время на порядок меньше. мне стало очевидно, что на некоторых сетах происходит слишком много таких
  вызовов, и я решил это место оптимизировать: я достаю элемент, делаю его свободным, но прежде чем его класть,
  я сразу смотрю на соседей, и если нужно, их тоже достаю _сразу_, объединяю и складываю (таким образом, 1 раз)
  После этого на этих тестах 90-91 программа стала проходить просто шикарно, в 10 раз быстрее. Стало понятно, что это и было
  горлышко. Однако теперь не ясно, как при такой реализации использовать функцию AppendIfFree. */
  void Free(Iterator position) {
    MemorySegment freed_memory_segment = MemorySegment(position->left, position->right);
    Iterator following_iterator = memory_segments_.erase(position);

    bool left_free = false, right_free = false;
    if (following_iterator != memory_segments_.end()) { 
      if (following_iterator->heap_index != MemorySegmentHeap::kNullIndex) {
        right_free = true;
      }
    }

    if (std::distance(memory_segments_.begin(), following_iterator) > 0) { 
      if (std::prev(following_iterator)->heap_index != MemorySegmentHeap::kNullIndex) {
        left_free = true;
      }
    }

    if (left_free && right_free) {
      MemorySegment united_memory_segment = 
        MemorySegment(std::prev(following_iterator)->left, following_iterator->right);
        free_memory_segments_.erase(following_iterator->heap_index);
        free_memory_segments_.erase(std::prev(following_iterator)->heap_index);
        memory_segments_.erase(std::prev(following_iterator));
        Iterator insert_iterator = memory_segments_.erase(following_iterator);

        memory_segments_.insert(insert_iterator, united_memory_segment);
        free_memory_segments_.push(std::prev(insert_iterator));
    } else {
      if (left_free) {
        MemorySegment united_memory_segment = 
        MemorySegment(std::prev(following_iterator)->left, freed_memory_segment.right);
        free_memory_segments_.erase(std::prev(following_iterator)->heap_index);
        Iterator insert_iterator = memory_segments_.erase(std::prev(following_iterator));

        memory_segments_.insert(insert_iterator, united_memory_segment);
        free_memory_segments_.push(std::prev(insert_iterator));
      }

      if (right_free) {
        MemorySegment united_memory_segment = 
        MemorySegment(freed_memory_segment.left, following_iterator->right);
        free_memory_segments_.erase(following_iterator->heap_index);
        Iterator insert_iterator = memory_segments_.erase(following_iterator);

        memory_segments_.insert(insert_iterator, united_memory_segment);
        free_memory_segments_.push(std::prev(insert_iterator));
      }
    }

    if (!left_free && !right_free) {
      memory_segments_.insert(following_iterator, freed_memory_segment);
      free_memory_segments_.push(std::prev(following_iterator));
    }
  }
  
  Iterator end() {
    return memory_segments_.end();
  }

  Iterator begin() {
    return memory_segments_.begin();
  }

  ConstIterator end() const {
    return memory_segments_.cend();
  }

 private:
  MemorySegmentHeap free_memory_segments_;
  std::list<MemorySegment> memory_segments_;

  void AppendIfFree(Iterator remaining, Iterator appending) {
    if (remaining->heap_index != MemorySegmentHeap::kNullIndex && 
      appending->heap_index != MemorySegmentHeap::kNullIndex) {
      MemorySegment merged_memory_segment = MemorySegment(remaining->left, appending->right);
      free_memory_segments_.erase(remaining->heap_index);
      free_memory_segments_.erase(appending->heap_index);

      memory_segments_.erase(remaining);
      Iterator following_iterator = memory_segments_.erase(appending);
      memory_segments_.insert(following_iterator, merged_memory_segment);   
      Iterator merged_memory_segment_iterator = --following_iterator;
      free_memory_segments_.push(merged_memory_segment_iterator);   
    }
  }
};

size_t ReadMemorySize(std::istream& stream = std::cin) {
  size_t memory_size;
  stream >> memory_size;
  return memory_size;
}

struct AllocationQuery {
  size_t allocation_size;
};

struct FreeQuery {
  int allocation_query_index;
};

class MemoryManagerQuery {
 public:
  explicit MemoryManagerQuery(AllocationQuery allocation_query) :
    query_(new ConcreteQuery<AllocationQuery>(allocation_query)) {
  }

  explicit MemoryManagerQuery(FreeQuery free_query) :
    query_(new ConcreteQuery<FreeQuery>(free_query)) {
  }

  const AllocationQuery* AsAllocationQuery() const {
    auto res = dynamic_cast<ConcreteQuery<AllocationQuery>*>(query_.get());
    return res ? &(res->body) : nullptr;
  }

  const FreeQuery* AsFreeQuery() const {
    auto res = dynamic_cast<ConcreteQuery<FreeQuery>*>(query_.get());
    return res ? &(res->body) : nullptr;
  }

 private:
  class AbstractQuery {
   public:
    virtual ~AbstractQuery() {
    }

   protected:
    AbstractQuery() {
    }
  };

  template <typename T>
  struct ConcreteQuery : public AbstractQuery {
    T body;

    explicit ConcreteQuery(T _body)
        : body(std::move(_body)) {
    }
  };

  std::unique_ptr<AbstractQuery> query_;
};

std::vector<MemoryManagerQuery> ReadMemoryManagerQueries(
    std::istream& stream = std::cin) {
  std::vector<MemoryManagerQuery> queries;
  int number_of_queries;
  stream >> number_of_queries;
  
  for (size_t index = 0; index < number_of_queries; ++index) {
    int query_initial_value;
    stream >> query_initial_value;
    
    if (query_initial_value > 0) {
      AllocationQuery allocation_query;
      allocation_query.allocation_size = static_cast<size_t>(query_initial_value);
      queries.push_back(MemoryManagerQuery(allocation_query));
    } else {
      FreeQuery free_query;
      free_query.allocation_query_index = - query_initial_value;
      queries.push_back(MemoryManagerQuery(free_query));
    }
  }

  return queries;
}

struct MemoryManagerAllocationResponse {
  bool success;
  size_t position;
};

MemoryManagerAllocationResponse MakeSuccessfulAllocation(size_t position) {
  MemoryManagerAllocationResponse response;
  response.success = true;
  response.position = position;
  return response;
}

MemoryManagerAllocationResponse MakeFailedAllocation() {
  MemoryManagerAllocationResponse response;
  response.success = false;
  response.position = size_t(-1);
  return response;
}

std::vector<MemoryManagerAllocationResponse> RunMemoryManager(
    size_t memory_size,
    const std::vector<MemoryManagerQuery>& queries) {
  std::vector<MemoryManagerAllocationResponse> responses;
  MemoryManager memory_manager(memory_size);
  std::vector<MemorySegmentIterator> allocation_iterators;
  size_t free_queries_passed = 0;
  std::vector<int> mapping;

  for (size_t query_index = 0; query_index < queries.size(); ++query_index) {
    const MemoryManagerQuery& query_iterator = queries[query_index];
    
    const AllocationQuery* allocation_query = query_iterator.AsAllocationQuery();
    mapping.push_back(query_index - free_queries_passed);
    if (allocation_query != nullptr) {
      MemorySegmentIterator allocation_iterator = 
      memory_manager.Allocate(allocation_query->allocation_size);
      allocation_iterators.push_back(allocation_iterator);
      MemoryManagerAllocationResponse response;

      if (allocation_iterator == memory_manager.end()) {
        response = MakeFailedAllocation();
      } else {
        response = MakeSuccessfulAllocation(allocation_iterator->left);
      }
      
      responses.push_back(response);
    } else {
      const FreeQuery* free_query = query_iterator.AsFreeQuery();
      ++free_queries_passed;
      MemorySegmentIterator to_free = 
      allocation_iterators[mapping[free_query->allocation_query_index - 1]];

      if (to_free != memory_manager.end()) {
        memory_manager.Free(to_free);
      }
    }
  }

  return responses;
}

void OutputMemoryManagerResponses(
    const std::vector<MemoryManagerAllocationResponse>& responses,
    std::ostream& ostream = std::cout) {
  for (auto iterator = responses.begin(); iterator != responses.end(); ++iterator) {
    if (iterator->success == true) {
      ostream << iterator->position + 1 << "\n";
    } else {
      ostream << -1 << "\n";
    }
  }
}

int main() {
  std::ios_base::sync_with_stdio(false);
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::istream& input_stream = std::cin;
  std::ostream& output_stream = std::cout;

  const size_t memory_size = ReadMemorySize(input_stream);
  const std::vector<MemoryManagerQuery> queries =
      ReadMemoryManagerQueries(input_stream);

  const std::vector<MemoryManagerAllocationResponse> responses =
      RunMemoryManager(memory_size, queries);

  OutputMemoryManagerResponses(responses, output_stream);

  return 0;
}
