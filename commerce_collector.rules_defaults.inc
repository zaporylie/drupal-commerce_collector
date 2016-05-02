<?php

/**
 * @file
 * Default rules configuration for commerce collector.
 */

/**
 * Implements hook_default_rules_configuration().
 */
function commerce_collector_default_rules_configuration() {
  $rules = array();

  $rules['commerce_collector_activate_payment_transaction'] = entity_import('rules_config', '{ "commerce_collector_activate_payment_transaction" : {
    "LABEL" : "Activate Collector Payment Transaction",
    "PLUGIN" : "reaction rule",
    "ACTIVE" : false,
    "TAGS" : [ "Commerce Payment" ],
    "REQUIRES" : [ "rules", "commerce_collector", "entity" ],
    "ON" : [ "commerce_order_update" ],
    "IF" : [
      { "data_is" : { "data" : [ "commerce-order:status" ], "value" : "completed" } },
      { "NOT data_is" : { "data" : [ "commerce-order-unchanged:status" ], "value" : "completed" } }
    ],
    "DO" : [
      { "commerce_collector_activate" : { "commerce_order" : [ "commerce-order" ] } }
    ]
  }
}
  ');
  $rules['commerce_collector_deactivate_payment_transaction'] = entity_import('rules_config', '{ "commerce_collector_deactivate_payment_transaction" : {
    "LABEL" : "Deactivate Collector Payment Transaction",
    "PLUGIN" : "reaction rule",
    "ACTIVE" : false,
    "TAGS" : [ "Commerce Payment" ],
    "REQUIRES" : [ "rules", "commerce_collector", "entity" ],
    "ON" : [ "commerce_order_update" ],
    "IF" : [
      { "data_is" : { "data" : [ "commerce-order:status" ], "value" : "canceled" } },
      { "NOT data_is" : { "data" : [ "commerce-order-unchanged:status" ], "value" : "canceled" } }
    ],
    "DO" : [
      { "commerce_collector_deactivate" : { "commerce_order" : [ "commerce-order" ] } }
    ]
  }
}
  ');

  return $rules;

}
