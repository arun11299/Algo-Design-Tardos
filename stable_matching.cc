#include <iostream>
#include <algorithm>
#include <vector>
#include <set>
#include <list>
#include <cassert>
#include <random>
#include <functional>
#include <iterator>

struct Entity
{
  Entity() = default;
  ~Entity() = default;

  bool is_free() const noexcept
  {
    return engaged_to_ == nullptr;
  }

  bool will_prefer(const Entity* e) const noexcept
  {
    auto itr1 = std::find_if(pref_entity_.begin(), pref_entity_.end(), 
                             std::bind2nd(std::equal_to<Entity*>(), e));

    if (itr1 == pref_entity_.end()) return false;

    auto itr2 = std::find_if(pref_entity_.begin(), pref_entity_.end(),
                             std::bind2nd(std::equal_to<Entity*>(), engaged_to_));

    return std::distance(pref_entity_.begin(), itr1) 
      < std::distance(pref_entity_.begin(), itr2);
  }

  void unmatch()
  {
    engaged_to_ = nullptr;
  }

  // Who the entity is engaged or linked to
  Entity* engaged_to_ = nullptr;
  // The preference list
  std::vector<Entity*> pref_entity_;
};

/*
 * Generates a list of random numbers
 */
std::vector<uint32_t> generate_random_set(uint32_t n, size_t ub)
{
  assert (n <= ub);
  std::vector<uint32_t> res;
  res.reserve(n);

  auto contains = [&](uint32_t value) {
    auto itr = std::find_if(std::begin(res), std::end(res), std::bind2nd(std::equal_to<uint32_t>(), value));
    return itr != std::end(res);
  };

  std::uniform_int_distribution<> dis(0, ub - 1);
  std::random_device rd;
  std::mt19937 gen(rd());

  while (res.size() < n) {
    auto v = dis(gen);
    std::cout << "Gen : " << v << std::endl;
    if (!contains(v)) {
      res.push_back(v);
    }
  }

  return res;
}

/*
 * Fill the preference list of Entity e with
 * n unique other Entities
 */
void fill_preflist(Entity& e, std::vector<Entity>& pool, uint32_t n)
{
  assert (n <= pool.size());
  auto rlist = generate_random_set(n, pool.size());

  e.pref_entity_.reserve(rlist.size());

  for (auto idx : rlist) {
    std::cout << "Idx : " << idx << std::endl;
    e.pref_entity_.push_back(&pool[idx]);
  }
}

void engage(Entity& a, Entity& b)
{
  if (not a.is_free()) {
    a.engaged_to_->unmatch();
    a.unmatch();
  }

  if (not b.is_free()) {
    b.engaged_to_->unmatch();
    b.unmatch();
  }

  a.engaged_to_ = &b;
  b.engaged_to_ = &a;
}

/**
 */
void bring_stability(std::vector<Entity>& set_a, std::vector<Entity>& set_b)
{
  std::set<Entity*> free_list;

  auto find_match = [&](Entity* pref, std::vector<Entity>& target) 
  {
    auto itr = std::find_if(target.begin(), target.end(), [&](const Entity& el) { return &el == pref; });
    return itr;
  };

  auto remove_from_free_list = [](std::set<Entity*>& s, Entity* to_del) mutable
  {
    auto itr = s.find(to_del);
    assert (itr != s.end());
    s.erase(itr);
  };


  for (auto & e_a : set_a) {
    free_list.insert(&e_a);
  }

  while (free_list.size()) {
    std::cout << free_list.size() << std::endl;

    std::uniform_int_distribution<> dis(0, free_list.size() - 1);
    std::random_device rd;
    std::mt19937 gen(rd());

    auto beg = free_list.begin();
    auto e_a = *std::next(beg, dis(gen));

    for (auto& pref : e_a->pref_entity_) {
      auto itr = find_match(pref, set_b);
      assert (itr != std::end(set_b) && "find_match should always succeed");
      // if the preference is free lock it
      if (itr->is_free()) {
        engage(*e_a, *itr);
        remove_from_free_list(free_list, e_a);
        break;
      } else if(itr->will_prefer(e_a)) {
        free_list.insert(itr->engaged_to_);
        engage(*e_a, *itr);
        remove_from_free_list(free_list, e_a);
        break;
      }
    }
  } // end while

}

int main() {
  std::vector<Entity> set_a; set_a.reserve(10);
  std::vector<Entity> set_b; set_b.reserve(10);

  std::uniform_int_distribution<> dis(0, set_a.size() - 1);
  std::random_device rd;
  std::mt19937 gen(rd());

  for (auto& e : set_a) fill_preflist(e, set_b, 10);
  for (auto& e : set_b) fill_preflist(e, set_a, 10);

  bring_stability(set_a, set_b);

  return 0;
}
