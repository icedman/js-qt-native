import { v4 as uuid } from "uuid";

const registry = {};

const formatJson = json => {
  let processed = { ...json };
  delete processed.children;
  delete processed.data;
  Object.keys(json).forEach(k => {
    if (typeof(json[k]) === 'function') {
      delete processed[k];
    }
  })
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
    delete registry[json.id];
  } catch (err) {}
};

const qt = {
  mount,
  unmount,
  update
};

window.$events = registry;

export default qt;
