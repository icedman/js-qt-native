import { v4 as uuid } from "uuid";

const registry = {};

const formatJson = json => {
  let processed = { ...json };
  if (json.style) {
    processed.style = { ...(processed.style || {}) };
  }
  // console.log(processed);
  delete processed.children;
  return JSON.stringify(processed);
};

const mount = json => {
  try {
    $qt.mount(formatJson(json));
  } catch (err) {}
};

const _events = ["onChangeText", "onClick", "onPress", "onRelease", "onSubmitEditing"];
const update = json => {
  try {
    $qt.update(formatJson(json));

    // events map events
    registry[json.id] = registry[json.id] || {};
    _events.forEach(e => {
      registry[json.id][e] = json[e] || ((evt) => {});
    });
  } catch (err) {}
};

const unmount = json => {
  try {
    $qt.unmount(formatJson(json));
    registry[json.id];
  } catch (err) {}
};

const qt = {
  mount,
  unmount,
  update
};

window.$events = registry;

export default qt;
