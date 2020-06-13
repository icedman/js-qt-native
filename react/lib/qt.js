
const registry = {};

const formatJson = json => {
  let processed = { ...json };
  if (json.style) {
    let style = { ...(processed.style || {}) };
    style.direction = style.flexDirection;
    style.flexDirection;
    processed.style = JSON.stringify(style);
  }
  delete processed.children;
  return JSON.stringify(processed);
};

const mount = (json) => {
    try {
    $qt.mount(formatJson(json));
    } catch(err) {}
}

const _events = [ 'onChange', 'onClick', 'onSubmit' ];
const update = (json) => {
    try {
    $qt.update(formatJson(json));

    // events map events
    registry[json.id] = registry[json.id] || {};
    _events.forEach(e => {
      registry[json.id][e] = json[e] || (()=>{});
    })

    } catch(err) {}
}

const unmount = (json) => {
    try {
    $qt.unmount(formatJson(json));
    registry[json.id];
    } catch(err) {}
}

const qt = {
    mount,
    unmount,
    update,
};

window.$events = registry;

export default qt;