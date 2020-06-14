import React from "react";
import View from "./view";

const FlatList = props => {
  return <View {...props} type="ScrollView" />;
};

export default FlatList;
