import React from "react";
import ReactDOM from "react-dom";
import { Window, View, SectionList, Text, StyleSheet } from "../../lib/core";

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
    <View style={styles.item} onPress={(evt)=>{console.log(title)}}>
      <Text style={styles.title}>{title}</Text>
    </View>
  );
}

function App() {
  return (
    <Window style={styles.container}>
      <View>
        <SectionList
          data={DATA}
          renderItem={({ item }) => <Item title={item} />}
          renderSectionHeader={({ section: { title } }) => (
            <Text style={styles.header}>{title}</Text>
          )}
          keyExtractor={(item, index) => item + index}
        />
      </View>
    </Window>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    // marginTop: Constants.statusBarHeight,
    marginHorizontal: 16
  },
  item: {
    backgroundColor: "#f9c2ff",
    padding: 20,
    marginVertical: 8
  },
  header: {
    fontSize: 32,
    backgroundColor: "#fff"
  },
  title: {
    fontSize: 24
  }
});


ReactDOM.render(<App />, document.getElementById("root"));
