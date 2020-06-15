import React from "react";
import ReactDOM from "react-dom";
import { v4 as uuid } from "uuid";

import qt from "../../lib/engine";
import {
  Window,
  View,
  Image,
  Text,
  TextInput,
  Button,
  ScrollView,
  FlatList,
  SectionList
} from "../../lib/core";

import { useTodos, StoreProvider as TodosProvider } from "./context";

const TodoItem = ({ item, toggleTodo, deleteTodo }) => {
  let Wrap = React.Fragment;
  if (item.checked) {
    Wrap = "s";
  }

  return (
    <View style={{ flexDirection: "row" }}>
      <Button
        text="check"
        // style={{ background: "red", border: "none" }}
        onClick={evt => {
          toggleTodo(item);
        }}
      />
      <Text style={{ flex: 2 }}>
        <Wrap>{item.text}</Wrap>
      </Text>
      <Button
        text="delete"
        onClick={evt => {
          deleteTodo(item);
        }}
      />
    </View>
  );
};

const App = () => {
  const todos = useTodos();
  const state = todos.state;

  const addTodo = evt => {
    if (state.newTodo === "") {
      return;
    }
    todos.dispatch(
      todos.setState({
        ...state,
        newTodo: "",
        todos: [
          ...state.todos,
          {
            id: uuid(),
            text: state.newTodo,
            checked: false
          }
        ]
      })
    );
  };

  const toggleTodo = todo => {
    let idx = state.todos.findIndex(t => {
      return t.id === todo.id;
    });

    if (idx !== -1) {
      let updateTodos = [...state.todos];
      updateTodos[idx].checked = !updateTodos[idx].checked;
      todos.dispatch(
        todos.setState({
          ...state,
          todos: updateTodos
        })
      );
    }
  };

  const deleteTodo = todo => {
    let idx = state.todos.findIndex(t => {
      return t.id === todo.id;
    });

    if (idx !== -1) {
      let updateTodos = [...state.todos];
      updateTodos.splice(idx, 1);
      todos.dispatch(
        todos.setState({
          ...state,
          todos: updateTodos
        })
      );
    }
  };

  const setNewTodo = evt => {
    todos.dispatch(
      todos.setState({
        ...state,
        newTodo: evt.target.value
      })
    );
  };

  const todosRendered = state.todos.map((todo, idx) => {
    let Wrap = React.Fragment;
    if (todo.checked) {
      Wrap = "s";
    }
    return (
      <View key={`todo-${idx}`} style={{ flexDirection: "row" }}>
        <Button
          text="check"
          // style={{ background: "red", border: "none" }}
          onClick={evt => {
            toggleTodo(todo);
          }}
        />
        <Text style={{ flex: 2 }}>
          <Wrap>{todo.text}</Wrap>
        </Text>
        <Button
          text="delete"
          onClick={evt => {
            deleteTodo(todo);
          }}
        />
      </View>
    );
  });

  return (
    <div>
      <Window id="mainWindow" style={{ flexDirection: "column" }}>
        <Text>
          <h2>Todos App</h2>
        </Text>
        <Image
          source="image-placeholder.jpg"
          style={{ width: 200, height: 200 }}
        />
        <View style={{ flexDirection: "row", flex: 0 }}>
          <TextInput
            text={state.newTodo}
            onSubmit={addTodo}
            onChange={setNewTodo}
          ></TextInput>
          <Button text="Add Todo" onClick={addTodo}></Button>
        </View>
        <ScrollView style={{ flex: 1 }}>
          <View style={{ flexDirection: "column", alignItems: "flex-start" }}>
            {todosRendered}
          </View>
        </ScrollView>
        <FlatList
          data={state.todos}
          renderItem={props => <TodoItem {...props} />}
          style={{ flex: 1 }}
          extraData={{ toggleTodo, deleteTodo }}
        ></FlatList>
      </Window>
    </div>
  );
};

ReactDOM.render(
  <TodosProvider>
    <App />
  </TodosProvider>,
  document.getElementById("root")
);
