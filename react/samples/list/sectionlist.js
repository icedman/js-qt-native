import React from "react";
import ReactDOM from "react-dom";
import { Window, View, SectionList, Text } from "../../lib/core";

const DATA = [
  {
    title: "Main dishes",
    data: ["Pizza", "Burger", "Risotto"]
  },
  {
    title: "Sides",
    data: ["French Fries", "Onion Rings", "Fried Shrimps"]
  },
  {
    title: "Drinks",
    data: ["Water", "Coke", "Beer"]
  },
  {
    title: "Desserts",
    data: ["Cheese Cake", "Ice Cream"]
  }
];

function Item({ title }) {
  return (
    <View>
      <Text>{title}</Text>
    </View>
  );
}

function App() {
  return (
    <Window>
      <View>
        <SectionList
          data={DATA}
          renderItem={({ item }) => <Item title={item} />}
          renderSectionHeader={({ section: { title } }) => (
            <Text>{title}</Text>
          )}
          keyExtractor={(item, index) => item + index}
        />
      </View>
    </Window>
  );
}

ReactDOM.render(<App />, document.getElementById("root"));
