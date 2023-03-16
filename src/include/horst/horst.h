#include <vector>
#include <horst/lart/junk.h>

namespace horst {
  typedef std::vector<float> port_buffer_t;
  typedef std::vector<port_buffer_t> port_buffers_t;

  struct unit {
    port_buffers_t m_port_buffers;

    virtual ~unit () {}
  };

  typedef std::shared_ptr<lart::junk<unit>> unit_ptr;

  struct ladspa_unit : public unit {

  };

  struct lv2_unit : public unit {

  };

  struct rack {
    std::vector<unit_ptr> units;
  };

  typedef std::shared_ptr<lart::junk<rack>> rack_ptr;

  struct horst {
    std::vector<rack_ptr> m_racks;
  };

  typedef std::shared_ptr<lart::junk<horst>> horst_ptr;
}
