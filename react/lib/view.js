import React from 'react';
import clsx from 'clsx';
import { v4 as uuid } from 'uuid';
import qt from './qt';

const View_ = props => {
  const [state, setState] = React.useState({
    type: props.type || "View",
    id: props.id || uuid(),
    parent: props.parent,
    persistent: props.id
  });

  let className = clsx('qt', props.type, props.className);
  let style = { display: 'flex', flexDirection:'column', ...(props.style || {}) };
  let uiInfo = { ...props, ...state, className: className };

  React.useEffect(() => {
    qt.mount(uiInfo);
    return () => {
      qt.unmount(uiInfo);
    };
  }, []);

  qt.update(uiInfo);

  let children = [];
  if (typeof props.children === "object") {
    {
      React.Children.map(props.children, (c, idx) => {
        if (!React.isValidElement(c)) {
          children.push(c);
          return;
        }
        children.push(
          React.cloneElement(
            c,
            { ...c.props, parent: state.id, key: `${state.id}-${idx}` },
            c.props.children
          )
        );
      });
    }
  }

  // preview on html only
  return (
    <div {...state} className={className} style={style}>
      {state.type}::{state.id} {children}
    </div>
  );
};

const View = React.memo(View_);

export default View;
