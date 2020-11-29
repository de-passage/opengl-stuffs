#include "moving_polygon.hpp"
#include "key_mapper.hpp"
#include "utils.hpp"

using namespace dpsg;
using namespace dpsg::input;

void moving_polygon(window &wdw) {
  key_mapper kmap;
  kmap.on(key::escape, close);
  wdw.set_key_callback(kmap);
  wdw.render_loop([] {

  });
}