#include <vector>

namespace horst {
  typedef std::vector<float> port_buffer_t;
  typedef std::vector<port_buffer_t> port_buffers_t;

  struct unit {
    port_buffers_t m_port_buffers;

    virtual ~unit () {}
  };

  struct ladspa_unit : public unit {

  };

  struct lv2_unit : public unit {

  };
}
