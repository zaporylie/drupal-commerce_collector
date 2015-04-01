<?php
/**
 * @file
 * Page callbacks accessible for users with administrative role only.
 */

/**
 * Module settings form.
 */
function commerce_collector_settings_form($form, &$form_state) {
  $form['commerce_collector_login'] = array(
    '#type' => 'fieldset',
    '#title' => t('Collector credentials'),
  );
  $form['commerce_collector_login']['commerce_collector_environment'] = array(
    '#type' => 'select',
    '#title' => t('Environment'),
    '#default_value' => variable_get('commerce_collector_environment', 'https://ecommercetest.collector.se/v3.0/InvoiceServiceV32.svc?singleWsdl'),
    '#options' => array(
      'https://ecommercetest.collector.se/v3.0/' => t('Test'),
      'https://ecommerce.collector.se/v3.0/' => t('Production'),
    ),
  );
  $form['commerce_collector_login']['commerce_collector_username'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#default_value' => variable_get('commerce_collector_username', NULL),
  );
  $form['commerce_collector_login']['commerce_collector_password'] = array(
    '#type' => 'textfield',
    '#title' => t('Password'),
    '#default_value' => variable_get('commerce_collector_password', NULL),
  );
  
  $form['commerce_collector_invoice'] = array(
    '#type' => 'fieldset',
    '#title' => t('Invoice settings'),
  );
  $form['commerce_collector_invoice']['commerce_collector_invoice_country'] = array(
    '#type' => 'select',
    '#title' => t('Available countries'),
    '#required' => TRUE,
    '#multiple' => TRUE,
    '#default_value' => variable_get('commerce_collector_invoice_country', array()),
    '#options' => array_intersect_key(country_get_list(), array_flip(_commerce_collector_get_countries())),
  );
  $form['commerce_collector_invoice']['commerce_collector_invoice_activation'] = array(
    '#type' => 'select',
    '#title' => t('Activation'),
    '#default_value' => variable_get('commerce_collector_invoice_activation', 0),
    '#options' => array(
      0 => t('Manual activation'),
      1 => t('Auto activation'),
      2 => t('Pre-paid (unavailable)'),
    ),
  );
  $form['commerce_collector_invoice']['commerce_collector_invoice_activation_state'] = array(
    '#type' => 'select',
    '#title' => t('Activate for state'),
    '#multiple' => TRUE,
    '#default_value' => variable_get('commerce_collector_invoice_activation_state', array('completed')),
    '#options' => commerce_order_state_options_list(),
    '#states' => array(
      'visible' => array(
        ':input[name="commerce_collector_invoice_activation"]' => array('value' => 0),
      ),
    ),
  );
  $form['commerce_collector_invoice']['commerce_collector_invoice_deactivation_state'] = array(
    '#type' => 'select',
    '#title' => t('Deactivate for state'),
    '#multiple' => TRUE,
    '#default_value' => variable_get('commerce_collector_invoice_deactivation_state', array('canceled')),
    '#options' => commerce_order_state_options_list(),
    '#states' => array(
      'visible' => array(
        ':input[name="commerce_collector_invoice_activation"]' => array('value' => 0),
      ),
    ),
  );
  $form['commerce_collector_invoice']['commerce_collector_invoice_delivery'] = array(
    '#type' => 'select',
    '#title' => t('Delivery method'),
    '#default_value' => variable_get('commerce_collector_invoice_delivery', 2),
    '#options' => array(
      1 => t('Normal mail or in package'),
      2 => t('E-mail'),
    ),
  );
  $form['commerce_collector_invoice']['commerce_collector_invoice_type'] = array(
    '#type' => 'select',
    '#title' => t('Invoice type'),
    '#default_value' => variable_get('commerce_collector_invoice_type', 0),
    '#options' => array(
      0 => t('Invoice in package or directly sent with e-mail'),
      1 => t('Monthly Invoice'),
      3 => t('Aggregated Invoice'),
      4 => t('Interest Free Invoice'),
      5 => t('Annuity invoice'),
    ),
  );
  
  $form['commerce_collector_vat'] = array(
    '#type' => 'fieldset',
    '#title' => t('VAT settings'),
  );
  $form['commerce_collector_vat']['commerce_collector_use_vat_module'] = array(
    '#type' => 'select',
    '#title' => t('Use VAT module'),
    '#description' => t('VAT rate for product items should be taken from commerce_vat module.'),
    '#default_value' => variable_get('commerce_collector_use_vat_module', TRUE),
    '#options' => array(
      TRUE => t('Yes'),
      FALSE => t('No'),
    ),
  );
  $form['commerce_collector_vat']['commerce_collector_vat_rate'] = array(
    '#type' => 'textfield',
    '#title' => t('Fixed VAT rate'),
    '#size' => 6,
    '#maxlength' => 6,
    '#default_value' => variable_get('commerce_collector_vat_rate', 0),
    '#element_validate' => array('_commerce_collector_validate_decimal_number'),
  );
  return system_settings_form($form);
}

function commerce_collector_get_invoice($order, $transaction) {
  $data = array(
    'CountryCode' => 'NO',
    'InvoiceNo' => $transaction->remote_id,
  );
  try {
    $result = _commerce_collector_request($data, 'GetCurrentInvoice', 'InformationService.svc', 'InformationService');
    $transaction->data['invoice'] = (array) $result;
    commerce_payment_transaction_save($transaction);
    drupal_set_message(t('Invoice information has been updated'));
  }
  catch (Exception $e) {
    drupal_set_message($e->getMessage(), 'error');
  }
  drupal_goto('admin/commerce/orders/' . $order->order_id . '/payment/' . $transaction->transaction_id);
}

/**
 * Validate decimal number.
 */
function _commerce_collector_validate_decimal_number($element, &$form_state) {
  $value = $element['#value'];
  $regexp = '@([^-0-9\\.])|(.-)@';
  if ($value != preg_replace($regexp, '', $value)) {
    form_error($element, t('Invalid number format. Should be decimal with . separator'));
  }
}
