import React from "react";
import ReactDOM from "react-dom";
import { v4 as uuid } from 'uuid';

import qt from '../../lib/qt';
import {
    MainWindow,
    View,
    Text,
    TextInput,
    Button
} from '../../lib/core';

import { useTodos, StoreProvider as TodosProvider } from './context';

const App = () => {
  const todos = useTodos();
  const state = todos.state;

  const addTodo = (evt) => {
    todos.dispatch(todos.setState({
      ...state,
      newTodo: '',
      todos: [ ...state.todos, {
        id: uuid(),
        text: state.newTodo,
        checked: false
      }]
    }));
  };

  const deleteTodo = (todo) => {
    let idx = state.todos.findIndex((t) => {
      return t.id === todo.id;
    });

    if (idx !== -1) {
      let updateTodos = [ ...state.todos ];
      updateTodos.splice(idx, 1);
      todos.dispatch(todos.setState({
        ...state,
        todos: updateTodos
      }));
    }
  }

  const setNewTodo = (evt) => {
    todos.dispatch(todos.setState({
        ...state,
        newTodo: evt.target.value
    }));
  }

  const todosRendered = state.todos.map((todo,idx) => {
    return <View key={`todo-${idx}`}>
        <Text text={todo.text}/>
        <Button text='delete' onClick={(evt) =>{ deleteTodo(todo) }}/>
      </View>
  })

    return <div>
        <MainWindow id='mainWindow' style={{flexDirection:'column'}}>
            <Text>
                <h2>Todoz App</h2>
            </Text>
            <View style={{flexDirection:'row', flex: 0}}>
              <TextInput text={state.newTodo} onSubmit={addTodo} onChange={setNewTodo}></TextInput>
              <Button text='Add Todo' onClick={addTodo}></Button>
            </View>
            <View style={{flexDirection:'column', flex: 1, alignItems: 'flex-start'}}>
            {todosRendered}
            </View>
        </MainWindow>
        </div>
}

ReactDOM.render(<TodosProvider><App/></TodosProvider>, document.getElementById("root"));